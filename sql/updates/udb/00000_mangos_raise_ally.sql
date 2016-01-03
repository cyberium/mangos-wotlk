-- Raise Ally
INSERT INTO `creature_template_spells` (`entry`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`) VALUES
(30230, 62225, 47480, 47481, 47482, 47484, 67886, 0);
UPDATE `mangos`.`creature_template` SET `MinLevel`='72', `MaxLevel`='80', `Expansion`='2', `MinLevelHealth`='9610', `MaxLevelHealth`='12600', `MinMeleeDmg`='131.91', `MaxMeleeDmg`='164.924', `MinRangedDmg`='131.91', `MaxRangedDmg`='164.924', `Armor`='6602', `MeleeAttackPower`='296', `RangedAttackPower`='40' WHERE (`Entry`='30230');

-- EventAI fir creature "Raise Ally" id: 30230
UPDATE creature_template SET AIName='EventAI' WHERE entry=30230;
DELETE FROM creature_ai_scripts WHERE creature_id=30230;
INSERT INTO creature_ai_scripts VALUES 
('3023001','30230','11','0','100','0','0','0','0','0','11','47448','0','2','11','47466','0','2','0','0','0','0','Risen Ally - Cast Risen Ghoul Spawn-In and Risen Ghoul Self Stun on Spawned');


