/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "WorldSocket.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <chrono>
#include "Common.h"
#include "Util.h"
#include "World.h"
#include "WorldPacket.h"
#include "SharedDefines.h"
#include "ByteBuffer.h"
#include "Opcodes.h"
#include "Database/DatabaseEnv.h"
#include "Auth/Sha1.h"
#include "WorldSession.h"
#include "WorldSocketMgr.h"
#include "Log.h"
#include "DBCStores.h"

using namespace std::chrono;

WorldSocket::WorldSocket(NetworkManager& socketMrg, NetworkThread& owner) :
    Socket(socketMrg, owner),
    m_packet(nullptr),
    received_header_(false),
    m_OverSpeedPings(0),
    m_session(0),
    m_seed(static_cast<uint32>(rand32()))
{
}

WorldSocket::~WorldSocket(void)
{
    if (m_packet != nullptr)
        delete m_packet;
}

void WorldSocket::CloseSocket(void)
{
    {
        GuardType Guard(m_sessionLock);
        m_session = NULL;
    }
    Socket::CloseSocket();
}

bool WorldSocket::SendPacket(const WorldPacket& pct)
{
    if (IsClosed())
        return false;

    // Dump outgoing packet.
    sLog.outWorldPacketDump(native_handle(), pct.GetOpcode(), pct.GetOpcodeName(), &pct, false);

    ServerPktHeader header(pct.size() + 2, pct.GetOpcode());
    m_crypt.EncryptSend((uint8*) header.header, header.getHeaderLength());

    GuardType Guard(m_outBufferLock);

    if (m_outBuffer->space() >= pct.size() + header.getHeaderLength())
    {
        // Put the packet on the buffer.
        if (!m_outBuffer->Write(header.header, header.getHeaderLength()))
            MANGOS_ASSERT(false);

        if (!pct.empty() && !m_outBuffer->Write(pct.contents(), pct.size()))
            MANGOS_ASSERT(false);
    }
    else
    {
        sLog.outError("network write buffer is too small to accommodate packet. Disconnecting client");
        return false;
    }

    StartAsyncSend();
    return true;
}

bool WorldSocket::Open()
{
   if (!Socket::Open())
       return false;

    // Send startup packet.
    WorldPacket packet(SMSG_AUTH_CHALLENGE, 40);
    packet << uint32(1);                                    // 1...31
    packet << m_seed;

    BigNumber seed1;
    seed1.SetRand(16 * 8);
    packet.append(seed1.AsByteArray(16), 16);               // new encryption seeds

    BigNumber seed2;
    seed2.SetRand(16 * 8);
    packet.append(seed2.AsByteArray(16), 16);               // new encryption seeds

    return SendPacket(packet);
}

bool WorldSocket::ProcessIncomingData()
{
    while (m_readBuffer->length() > 0)
    {
        if (!received_header_)
        {
            if (!ReadPacketHeader())
                return true;

            if (!ValidatePacketHeader())
                return false;
        }

        if (!ReadPacketContent())
            return true;

        if (!ProcessPacket(m_packet))
            return false;
    }

    return true;
}

bool WorldSocket::ReadPacketHeader()
{
    if (m_readBuffer->Read((uint8*)&m_header, CLIENT_PACKET_HEADER_SIZE))
    {
        received_header_ = true;
        return true;
    }

    return false;
}

bool WorldSocket::ValidatePacketHeader()
{
    m_crypt.DecryptRecv((uint8*) &m_header, CLIENT_PACKET_HEADER_SIZE);

    m_header.Convert();

    if (m_header.IsValid())
    {
        m_header.size -= 4;
        m_packet = new WorldPacket((Opcodes) m_header.cmd, m_header.size);

        if (m_header.size > 0)
            m_packet->resize(m_header.size);
        else
            MANGOS_ASSERT(m_packet->size() == 0);

        return true;
    }
    else
        sLog.outError("WorldSocket::ValidatePacketHeader: Client sent malformed packet size = %d , cmd = %d", m_header.size, m_header.cmd);

    return false;
}

bool WorldSocket::ReadPacketContent()
{
    MANGOS_ASSERT(m_packet != nullptr);

    if (m_packet->size() > 0)
    {
        if (!m_readBuffer->Read((uint8*) m_packet->contents(), m_packet->size()))
            return false;
    }

    received_header_ = false;
    return true;
}

bool WorldSocket::ProcessPacket(WorldPacket* newPkt)
{
    // manage memory ;)
    std::auto_ptr<WorldPacket> aptr(newPkt);

    const uint16 opcode = newPkt->GetOpcode();

    if (opcode >= NUM_MSG_TYPES)
    {
        sLog.outError("SESSION: received nonexistent opcode 0x%.4X", opcode);
        return false;
    }

    if (IsClosed())
        return false;

    // Dump received packet.
    sLog.outWorldPacketDump(native_handle(), newPkt->GetOpcode(), newPkt->GetOpcodeName(), newPkt, true);

    try
    {
        switch (opcode)
        {
            case CMSG_PING:
                return HandlePing(*newPkt);
            case CMSG_AUTH_SESSION:
                if (m_session)
                {
                    sLog.outError("WorldSocket::ProcessIncoming: Player send CMSG_AUTH_SESSION again");
                    return false;
                }
                return HandleAuthSession(*newPkt);
            case CMSG_KEEP_ALIVE:
                DEBUG_LOG("CMSG_KEEP_ALIVE ,size: " SIZEFMTD " ", newPkt->size());
                return true;
            default:
            {
                GuardType Guard(m_sessionLock);

                if (m_session != NULL)
                {
                    // OK ,give the packet to WorldSession
                    aptr.release();
                    // WARNING here we call it with locks held.
                    // Its possible to cause deadlock if QueuePacket calls back
                    m_session->QueuePacket(newPkt);
                    return true;
                }
                else
                {
                    sLog.outError("WorldSocket::ProcessIncoming: Client not authed opcode = %u", uint32(opcode));
                    return false;
                }
            }
        }
    }
    catch (ByteBufferException&)
    {
        sLog.outError("WorldSocket::ProcessIncoming ByteBufferException occured while parsing an instant handled packet (opcode: %u) from client %s, accountid=%i.",
                      opcode, GetRemoteAddress().c_str(), m_session ? m_session->GetAccountId() : -1);

        if (sLog.HasLogLevelOrHigher(LOG_LVL_DEBUG))
        {
            DEBUG_LOG("Dumping error-causing packet:");
            newPkt->hexlike();
        }

        if (sWorld.getConfig(CONFIG_BOOL_KICK_PLAYER_ON_BAD_PACKET))
        {
            DETAIL_LOG("Disconnecting session [account id %i / address %s] for badly formatted packet.",
                       m_session ? m_session->GetAccountId() : -1, GetRemoteAddress().c_str());
            return false;
        }
        else
            return 0;
    }

    m_packet = nullptr;

    return true;
}

bool WorldSocket::HandleAuthSession(WorldPacket& recvPacket)
{
    // NOTE: ATM the socket is singlethread, have this in mind ...
    uint8 digest[20];
    uint32 clientSeed, id, security;
    uint32 ClientBuild;
    uint8 expansion = 0;
    LocaleConstant locale;
    std::string account;
    Sha1Hash sha1;
    BigNumber v, s, g, N, K;
    WorldPacket packet;

    // Read the content of the packet
    recvPacket >> ClientBuild;
    recvPacket.read_skip<uint32>();
    recvPacket >> account;
    recvPacket.read_skip<uint32>();
    recvPacket >> clientSeed;
    recvPacket.read_skip<uint32>();
    recvPacket.read_skip<uint32>();
    recvPacket.read_skip<uint32>();
    recvPacket.read_skip<uint64>();
    recvPacket.read(digest, 20);

    DEBUG_LOG("WorldSocket::HandleAuthSession: client build %u, account %s, clientseed %X", ClientBuild, account.c_str(), clientSeed);

    // Check the version of client trying to connect
    if (!IsAcceptableClientBuild(ClientBuild))
    {
        packet.Initialize(SMSG_AUTH_RESPONSE, 1);
        packet << uint8(AUTH_VERSION_MISMATCH);

        SendPacket(packet);

        sLog.outError("WorldSocket::HandleAuthSession: Sent Auth Response (version mismatch).");
        return false;
    }

    // Get the account information from the realmd database
    std::string safe_account = account; // Duplicate, else will screw the SHA hash verification below
    LoginDatabase.escape_string(safe_account);
    // No SQL injection, username escaped.

    QueryResult* result =
        LoginDatabase.PQuery("SELECT "
                             "id, "                      //0
                             "gmlevel, "                 //1
                             "sessionkey, "              //2
                             "last_ip, "                 //3
                             "locked, "                  //4
                             "v, "                       //5
                             "s, "                       //6
                             "expansion, "               //7
                             "mutetime, "                //8
                             "locale "                   //9
                             "FROM account "
                             "WHERE username = '%s'",
                             safe_account.c_str());

    // Stop if the account is not found
    if (!result)
    {
        packet.Initialize(SMSG_AUTH_RESPONSE, 1);
        packet << uint8(AUTH_UNKNOWN_ACCOUNT);

        SendPacket(packet);

        sLog.outError("WorldSocket::HandleAuthSession: Sent Auth Response (unknown account).");
        return false;
    }

    Field* fields = result->Fetch();

    expansion = ((sWorld.getConfig(CONFIG_UINT32_EXPANSION) > fields[7].GetUInt8()) ? fields[7].GetUInt8() : sWorld.getConfig(CONFIG_UINT32_EXPANSION));

    N.SetHexStr("894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7");
    g.SetDword(7);

    v.SetHexStr(fields[5].GetString());
    s.SetHexStr(fields[6].GetString());
    m_sessionKey = s;

    const char* sStr = s.AsHexStr();                        // Must be freed by OPENSSL_free()
    const char* vStr = v.AsHexStr();                        // Must be freed by OPENSSL_free()

    DEBUG_LOG("WorldSocket::HandleAuthSession: (s,v) check s: %s v: %s",
              sStr,
              vStr);

    OPENSSL_free((void*) sStr);
    OPENSSL_free((void*) vStr);

    ///- Re-check ip locking (same check as in realmd).
    if (fields[4].GetUInt8() == 1)  // if ip is locked
    {
        if (strcmp(fields[3].GetString(), GetRemoteAddress().c_str()) != 0)
        {
            packet.Initialize(SMSG_AUTH_RESPONSE, 1);
            packet << uint8(AUTH_FAILED);
            SendPacket(packet);

            delete result;
            BASIC_LOG("WorldSocket::HandleAuthSession: Sent Auth Response (Account IP differs).");
            return false;
        }
    }

    id = fields[0].GetUInt32();
    security = fields[1].GetUInt16();
    if (security > SEC_ADMINISTRATOR)                       // prevent invalid security settings in DB
        security = SEC_ADMINISTRATOR;

    K.SetHexStr(fields[2].GetString());

    time_t mutetime = time_t (fields[8].GetUInt64());

    locale = LocaleConstant(fields[9].GetUInt8());
    if (locale >= MAX_LOCALE)
        locale = LOCALE_enUS;

    delete result;

    // Re-check account ban (same check as in realmd)
    QueryResult* banresult =
        LoginDatabase.PQuery("SELECT 1 FROM account_banned WHERE id = %u AND active = 1 AND (unbandate > UNIX_TIMESTAMP() OR unbandate = bandate)"
                             "UNION "
                             "SELECT 1 FROM ip_banned WHERE (unbandate = bandate OR unbandate > UNIX_TIMESTAMP()) AND ip = '%s'",
                             id, GetRemoteAddress().c_str());

    if (banresult) // if account banned
    {
        packet.Initialize(SMSG_AUTH_RESPONSE, 1);
        packet << uint8(AUTH_BANNED);
        SendPacket(packet);

        delete banresult;

        sLog.outError("WorldSocket::HandleAuthSession: Sent Auth Response (Account banned).");
        return false;
    }

    // Check locked state for server
    AccountTypes allowedAccountType = sWorld.GetPlayerSecurityLimit();

    if (allowedAccountType > SEC_PLAYER && AccountTypes(security) < allowedAccountType)
    {
        packet.Initialize(SMSG_AUTH_RESPONSE, 1);
        packet << uint8(AUTH_UNAVAILABLE);

        SendPacket(packet);

        BASIC_LOG("WorldSocket::HandleAuthSession: User tries to login but his security level is not enough");
        return false;
    }

    // Check that Key and account name are the same on client and server
    Sha1Hash sha;

    uint32 t = 0;
    uint32 seed = m_seed;

    sha.UpdateData(account);
    sha.UpdateData((uint8*) & t, 4);
    sha.UpdateData((uint8*) & clientSeed, 4);
    sha.UpdateData((uint8*) & seed, 4);
    sha.UpdateBigNumbers(&K, NULL);
    sha.Finalize();

    if (memcmp(sha.GetDigest(), digest, 20) != 0)
    {
        packet.Initialize(SMSG_AUTH_RESPONSE, 1);
        packet << uint8(AUTH_FAILED);

        SendPacket(packet);

        sLog.outError("WorldSocket::HandleAuthSession: Sent Auth Response (authentification failed).");
        return false;
    }

    std::string address = GetRemoteAddress();

    DEBUG_LOG("WorldSocket::HandleAuthSession: Client '%s' authenticated successfully from %s.",
              account.c_str(),
              address.c_str());

    // Update the last_ip in the database
    // No SQL injection, username escaped.
    static SqlStatementID updAccount;

    SqlStatement stmt = LoginDatabase.CreateStatement(updAccount, "UPDATE account SET last_ip = ? WHERE username = ?");
    stmt.PExecute(address.c_str(), account.c_str());

    WorldSocketPtr this_session = boost::static_pointer_cast<WorldSocket>(shared_from_this());
    // NOTE ATM the socket is single-threaded, have this in mind ...
    m_session = new WorldSession(id, this_session, AccountTypes(security), expansion, mutetime, locale);

    m_crypt.Init(&K);

    m_session->LoadGlobalAccountData();
    m_session->LoadTutorialsData();
    m_session->ReadAddonsInfo(recvPacket);

    boost::this_thread::sleep(boost::posix_time::milliseconds(10));

    sWorld.AddSession(m_session);

    return true;
}

bool WorldSocket::HandlePing(WorldPacket& recvPacket)
{
    uint32 ping;
    uint32 latency;

    // Get the ping packet content
    recvPacket >> ping;
    recvPacket >> latency;

    if (m_LastPingTime.time_since_epoch().count() == 0)
        m_LastPingTime = system_clock::now();            // for 1st ping
    else
    {
        system_clock::time_point cur_time = system_clock::now();
        system_clock::duration diff_time = cur_time - m_LastPingTime;
        m_LastPingTime = cur_time;

        if (diff_time < seconds(27))
        {
            ++m_OverSpeedPings;

            uint32 max_count = sWorld.getConfig(CONFIG_UINT32_MAX_OVERSPEED_PINGS);

            if (max_count && m_OverSpeedPings > max_count)
            {
                GuardType Guard(m_sessionLock);

                if (m_session && m_session->GetSecurity() == SEC_PLAYER)
                {
                    sLog.outError("WorldSocket::HandlePing: Player kicked for "
                                  "overspeeded pings address = %s",
                                  GetRemoteAddress().c_str());

                    return false;
                }
            }
        }
        else
            m_OverSpeedPings = 0;
    }

    // critical section
    {
        GuardType Guard(m_sessionLock);

        if (m_session)
            m_session->SetLatency(latency);
        else
        {
            sLog.outError("WorldSocket::HandlePing: peer sent CMSG_PING, "
                          "but is not authenticated or got recently kicked,"
                          " address = %s",
                          GetRemoteAddress().c_str());
            return false;
        }
    }

    WorldPacket packet(SMSG_PONG, 4);
    packet << ping;

    if (!SendPacket(packet))
        return false;

    return true;
}
