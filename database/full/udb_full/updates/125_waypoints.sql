-- Theramore -- Update
-- Sergeant Amelyn -- waypoints
-- Theramore Guards came back frome future (cata) -- and now have correct (wotlk) waypoints
-- Missing ones added
-- also small update for: Stitched Horror - correct spawn

-- Sergeant Amelyn -- waypoints
UPDATE creature SET MovementType = 2, spawndist = 0 WHERE guid = 18589;
DELETE FROM creature_movement WHERE id = 18589;
UPDATE creature_template SET MovementType = 2 WHERE entry = 23835;
DELETE FROM creature_movement_template WHERE entry = 23835;
INSERT INTO creature_movement_template (entry, point, position_x, position_y, position_z, waittime, script_id, orientation, model1, model2) VALUES
(23835,1,-3845.5,-4561.34,8.42438,0,0,5.89954,0,0),(23835,2,-3838.69,-4562.43,8.51284,0,0,0.255656,0,0),(23835,3,-3834.04,-4560.31,8.67834,0,0,0.699406,0,0),
(23835,4,-3831.14,-4555.43,8.90911,0,0,1.3183,0,0),(23835,5,-3832.25,-4547.09,9.17283,0,0,2.25921,0,0),(23835,6,-3837.1,-4542.97,9.21158,0,0,2.65074,0,0),
(23835,7,-3843.13,-4541.69,9.09676,0,0,3.36152,0,0),(23835,8,-3849.58,-4544.78,8.78854,0,0,3.84454,0,0),(23835,9,-3852.47,-4549.77,8.59729,0,0,4.50507,0,0),
(23835,10,-3850.63,-4556.64,8.43525,0,0,5.49623,0,0),(23835,11,-3846.7,-4558.22,8.51499,0,0,5.51351,0,0),(23835,12,-3846.7,-4558.22,8.51499,150000,0,3.89208,0,0);

-- Theramore Guards -- spawn point corrected
UPDATE creature SET position_x = -3864.047119, position_y = -4574.887207, position_z = 8.050746, orientation = 3.975231, spawndist = 15, MovementType = 1 WHERE guid = 30656;

-- Theramore Guards should work in pairs
-- 30634 and 30634 - wrong wps fixed. + link 
UPDATE creature SET spawndist = 0, MovementType = 2 WHERE guid = 30634;
UPDATE creature SET spawndist = 0, MovementType = 0 WHERE guid = 30633;
DELETE FROM creature_movement WHERE id IN (30634,30633);
INSERT INTO creature_movement (id, point, position_x, position_y, position_z, waittime, script_id, orientation, model1, model2) VALUES
(30634,1,-3618.87,-4338.46,8.92569,0,0,0.670259,0,0),(30634,2,-3605.54,-4327.62,9.43362,0,0,0.685966,0,0),(30634,3,-3595.24,-4319,7.52476,3000,0,0.639628,0,0),
(30634,4,-3596.5,-4325.37,8.36596,0,0,3.70033,0,0),(30634,5,-3611.79,-4337.68,9.46105,0,0,3.80636,0,0),(30634,6,-3623.07,-4346.82,7.74394,0,0,3.80089,0,0),
(30634,7,-3629.41,-4351.74,5.99012,0,0,3.80089,0,0),(30634,8,-3651.49,-4369.54,9.3791,2000,0,3.8417,0,0),(30634,9,-3649.29,-4363.62,9.23098,0,0,0.660052,0,0),
(30634,10,-3631.88,-4349.01,6.05115,0,0,0.692253,0,0);
DELETE FROM creature_linking WHERE guid = 30633;
INSERT INTO creature_linking (guid, master_guid, flag) VALUES
(30633, 30634, 512);
-- 30644 and 30643 - wrong wps fixed + link
UPDATE creature SET position_x = -3684.833740, position_y = -4475.191895, position_z = 11.761748, orientation = 0.844338 WHERE guid = 30643;
UPDATE creature SET spawndist = 0, MovementType = 2 WHERE guid = 30643;
UPDATE creature SET spawndist = 0, MovementType = 0 WHERE guid = 30644;
DELETE FROM creature_movement WHERE id IN (30643,30644);
INSERT INTO creature_movement (id, point, position_x, position_y, position_z, waittime, script_id, orientation, model1, model2) VALUES
(30643,1,-3679.7,-4468.75,11.5796,1000,497901,1.15329,0,0),(30643,2,-3675.39,-4456.55,11.4846,0,0,1.47138,0,0),(30643,3,-3673.94,-4419.96,11.2135,0,0,1.54599,0,0),
(30643,4,-3674.24,-4400.51,10.7088,0,0,1.73213,0,0),(30643,5,-3675.96,-4394.77,10.624,0,0,2.19316,0,0),(30643,6,-3680.25,-4392.75,10.6125,0,0,3.02568,0,0),
(30643,7,-3696.75,-4392.7,13.5437,0,0,3.12464,0,0),(30643,8,-3703.1,-4392.73,15.9825,0,0,3.57782,0,0),(30643,9,-3707.81,-4395.67,17.7928,0,0,3.92187,0,0),
(30643,10,-3711.49,-4401.82,20.4451,0,0,4.24388,0,0),(30643,11,-3714.61,-4409.16,23.324,0,0,4.08602,0,0),(30643,12,-3721.77,-4415.38,26.1858,3000,0,3.83941,0,0),
(30643,13,-3720.04,-4409.84,24.4707,0,0,0.827402,0,0),(30643,14,-3716.21,-4404.82,22.4493,0,0,1.12585,0,0),(30643,15,-3713.1,-4397.83,19.645,0,0,0.982126,0,0),
(30643,16,-3709.22,-4392.7,17.7584,0,0,0.61613,0,0),(30643,17,-3704.55,-4390.33,16.2071,0,0,0.273697,0,0),(30643,18,-3696.17,-4389.93,13.1989,0,0,6.25843,0,0),
(30643,19,-3689.48,-4389.6,11.1495,0,0,0.27998,0,0),(30643,20,-3682.87,-4387.13,10.6097,0,0,0.118188,0,0),(30643,21,-3676.26,-4387.96,10.4594,0,0,5.65368,0,0),
(30643,22,-3671.9,-4393.95,10.5895,0,0,4.95546,0,0),(30643,23,-3670.12,-4407.01,10.613,0,0,4.78111,0,0),(30643,24,-3670.61,-4434.57,11.2723,0,0,4.65937,0,0),
(30643,25,-3671.58,-4453.53,11.3598,0,0,4.48344,0,0),(30643,26,-3675.29,-4467.09,11.3622,0,0,4.17007,0,0),(30643,27,-3681.77,-4477.23,11.4931,0,0,3.91561,0,0),
(30643,28,-3691.84,-4485.12,11.9509,0,0,3.68313,0,0),(30643,29,-3704.93,-4492,12.2934,0,0,3.56532,0,0),(30643,30,-3722.11,-4499.53,12.6519,0,0,3.52212,0,0),
(30643,31,-3737.38,-4504.39,12.7619,0,0,3.41217,0,0),(30643,32,-3755.25,-4509.12,12.5852,0,0,3.41452,0,0),(30643,33,-3771.1,-4512.03,12.2303,0,0,3.3242,0,0),
(30643,34,-3795.2,-4513.99,11.5415,0,0,3.46636,0,0),(30643,35,-3807.35,-4521.96,9.75797,0,0,3.91168,0,0),(30643,36,-3822.92,-4537.21,9.21865,0,0,3.91718,0,0),
(30643,37,-3830.78,-4545.17,9.21865,0,0,4.06797,0,0),(30643,38,-3830.35,-4551.46,9.06977,0,0,4.6688,0,0),(30643,39,-3831.51,-4558.27,8.85811,0,0,4.346,0,0),
(30643,40,-3836.16,-4563.35,8.56187,0,0,3.5881,0,0),(30643,41,-3843.31,-4564.34,8.37255,0,0,3.01683,0,0),(30643,42,-3850.33,-4561.42,8.30779,0,0,2.45291,0,0),
(30643,43,-3854.74,-4554.54,8.38975,0,0,1.717,0,0),(30643,44,-3852.95,-4546.57,8.63064,0,0,1.10351,0,0),(30643,45,-3848.58,-4542.12,9.00319,0,0,0.421784,0,0),
(30643,46,-3840.52,-4541.22,9.1465,0,0,6.15755,0,0),(30643,47,-3835.21,-4543.37,9.18398,0,0,0.279628,0,0),(30643,48,-3825.16,-4533.75,9.21787,0,0,0.79485,0,0),
(30643,49,-3809.29,-4519.13,9.85881,0,0,0.649551,0,0),(30643,50,-3800.73,-4512.63,11.4626,0,0,0.429639,0,0),(30643,51,-3793.88,-4509.93,11.5422,0,0,0.0589311,0,0),
(30643,52,-3777.41,-4509.2,12.2954,0,0,0.122582,0,0),(30643,53,-3755.42,-4504.67,12.9063,0,0,0.17756,0,0),(30643,54,-3731.24,-4498.58,13.0048,0,0,0.236465,0,0),
(30643,55,-3716.4,-4493.13,12.7937,0,0,0.377837,0,0),(30643,56,-3701.82,-4486.44,12.3934,0,0,0.436742,0,0),(30643,57,-3691.81,-4480.81,12.0252,0,0,0.625237,0,0),
(30643,58,-3685.02,-4475.38,11.7683,1000,497902,0.774463,0,0);
DELETE FROM creature_linking WHERE guid = 30644;
INSERT INTO creature_linking (guid, master_guid, flag) VALUES
(30644, 30643, 512);
-- 30638 and 30639 - wrong wps fixed + link
UPDATE creature SET position_x = -3692.260010, position_y = -4377.211914, position_z = 10.612564, orientation = 5.529586 WHERE guid = 30638;
UPDATE creature SET spawndist = 0, MovementType = 0 WHERE guid = 30638;
UPDATE creature SET spawndist = 0, MovementType = 2 WHERE guid = 30639;
DELETE FROM creature_movement WHERE id IN (30638,30639);
INSERT INTO creature_movement (id, point, position_x, position_y, position_z, waittime, script_id, orientation, model1, model2) VALUES
(30639,1,-3680.76,-4381.97,10.5645,1000,497901,5.72122,0,0),(30639,2,-3673.04,-4391.08,10.5271,0,0,5.05442,0,0),(30639,3,-3669.26,-4409.72,10.6197,0,0,4.77561,0,0),
(30639,4,-3669.39,-4431.5,11.264,0,0,4.68529,0,0),(30639,5,-3671.5,-4454.84,11.3452,0,0,4.66094,0,0),(30639,6,-3675.31,-4467.79,11.3624,0,0,4.14415,0,0),
(30639,7,-3687.56,-4481.97,11.7599,0,0,3.86063,0,0),(30639,8,-3706.28,-4492.55,12.3353,0,0,3.57003,0,0),(30639,9,-3727.55,-4501.54,12.7,0,0,3.37761,0,0),
(30639,10,-3749.83,-4508,12.6542,0,0,3.4208,0,0),(30639,11,-3768.59,-4511.94,12.275,0,0,3.33441,0,0),(30639,12,-3784.12,-4513.69,11.8161,0,0,3.22053,0,0),
(30639,13,-3791.71,-4513.16,11.5189,0,0,2.81605,0,0),(30639,14,-3798.05,-4508.75,11.5403,0,0,2.26705,0,0),(30639,15,-3806.97,-4489.25,11.9206,0,0,1.96389,0,0),
(30639,16,-3814.24,-4470.58,12.6997,0,0,1.80759,0,0),(30639,17,-3814.65,-4447.15,12.7041,0,0,1.54527,0,0),(30639,18,-3814.78,-4427.61,12.7665,0,0,1.60025,0,0),
(30639,19,-3811.63,-4414.81,12.7831,0,0,1.23504,0,0),(30639,20,-3804.17,-4401.99,11.7984,0,0,0.700967,0,0),(30639,21,-3790.15,-4393.62,10.4463,0,0,0.286277,0,0),
(30639,22,-3766.9,-4387.99,10.8432,0,0,0.422151,0,0),(30639,23,-3735.79,-4371.46,10.6132,0,0,0.492837,0,0),(30639,24,-3720.68,-4364.3,10.6123,0,0,0.313766,0,0),
(30639,25,-3706.65,-4364.11,10.613,0,0,5.8901,0,0),(30639,26,-3691.2,-4375.61,10.6133,1000,497902,5.81774,0,0);
DELETE FROM creature_linking WHERE guid = 30638;
INSERT INTO creature_linking (guid, master_guid, flag) VALUES
(30638, 30639, 512);
-- 54310 and 54571 -- missing added + link
-- UDB empty guids reused
DELETE FROM creature WHERE guid IN (54310,54571);
INSERT INTO creature (guid, id, map, spawnMask, phaseMask, modelid, equipment_id, position_x, position_y, position_z, orientation, spawntimesecs, spawndist, currentwaypoint, curhealth, curmana, DeathState, MovementType) VALUES 
(54310,4979,1,1,1,0,0,-3807.06,-4479.2,12.6748,5.12423,360,0,0,5071,0,0,2),
(54571,4979,1,1,1,0,0,-3808.67,-4479.9,12.6631,5.12423,360,0,0,5071,0,0,0);
DELETE FROM creature_movement WHERE id IN (54310,54571);
INSERT INTO creature_movement (id, point, position_x, position_y, position_z, waittime, script_id, orientation, model1, model2) VALUES
(54310,1,-3800.44,-4494.17,11.5613,1000,497901,5.11638,0,0),(54310,2,-3797.36,-4503.56,11.5546,0,0,4.77001,0,0),(54310,3,-3798.16,-4511.52,11.5428,0,0,4.30035,0,0),
(54310,4,-3805.8,-4520.85,9.95399,0,0,3.96577,0,0),(54310,5,-3822.64,-4538.08,9.21827,0,0,3.93435,0,0),(54310,6,-3830.74,-4544.93,9.21827,0,0,3.76785,0,0),
(54310,7,-3836.15,-4546.38,9.25034,0,0,2.96046,0,0),(54310,8,-3842.03,-4544.86,9.22394,0,0,3.08933,0,0),(54310,9,-3847.31,-4546.23,8.90135,0,0,3.79383,0,0),
(54310,10,-3850.49,-4551.63,8.63787,0,0,4.67868,0,0),(54310,11,-3848.9,-4557.78,8.43152,0,0,5.3981,0,0),(54310,12,-3843.6,-4561.7,8.45701,0,0,6.18977,0,0),
(54310,13,-3836.59,-4560.35,8.69577,0,0,0.54119,0,0),(54310,14,-3832.98,-4555.53,8.8495,0,0,1.20407,0,0),(54310,15,-3832.91,-4548.91,9.08172,0,0,1.73028,0,0),
(54310,16,-3831.78,-4542.91,9.21922,0,0,0.994365,0,0),(54310,17,-3825.16,-4534.65,9.21922,0,0,0.721831,0,0),(54310,18,-3802.37,-4512.15,11.3335,3000,0,0.803513,0,0),
(54310,19,-3801.35,-4504.08,11.573,0,0,1.7185,0,0),(54310,20,-3804.61,-4493.02,11.66,0,0,1.88736,0,0),(54310,21,-3811.27,-4476.45,12.6976,0,0,1.75542,0,0),
(54310,22,-3813.51,-4466.46,12.6997,0,0,1.62347,0,0),(54310,23,-3813.86,-4450.22,12.7001,0,0,1.58027,0,0),(54310,24,-3813.57,-4428.9,12.7114,0,0,1.43026,0,0),
(54310,25,-3811.28,-4417.94,12.7054,0,0,1.13809,0,0),(54310,26,-3804.17,-4404.95,12.0706,0,0,0.908756,0,0),(54310,27,-3795.55,-4396.78,10.7908,0,0,0.428092,0,0),
(54310,28,-3783.55,-4392.79,10.6013,0,0,0.251378,0,0),(54310,29,-3764.57,-4388,10.7648,0,0,0.395891,0,0),(54310,30,-3744.72,-4376.1,10.6153,0,0,0.549044,0,0),
(54310,31,-3722.36,-4366.04,10.6126,0,0,0.415526,0,0),(54310,32,-3718.78,-4363.55,10.6126,0,0,0.970803,0,0),(54310,33,-3717.35,-4355.41,10.6126,0,0,1.50723,0,0),
(54310,34,-3716.43,-4347.11,10.6126,0,0,1.2504,0,0),(54310,35,-3712.34,-4341.61,10.6126,4000,0,0.890489,0,0),(54310,36,-3715.46,-4364.03,10.6124,0,0,4.14754,0,0),
(54310,37,-3722.47,-4368.69,10.6124,0,0,3.7428,0,0),(54310,38,-3743.64,-4379.64,10.6884,0,0,3.60536,0,0),(54310,39,-3763.98,-4390.51,10.7454,0,0,3.49147,0,0),
(54310,40,-3791.73,-4398.1,10.5897,0,0,3.37759,0,0),(54310,41,-3800.01,-4403.57,11.464,0,0,3.85904,0,0),(54310,42,-3808.22,-4416.12,12.7012,0,0,4.39704,0,0),
(54310,43,-3810.66,-4428.11,12.7104,0,0,4.78345,0,0),(54310,44,-3811.06,-4452.62,12.7018,0,0,4.68135,0,0),(54310,45,-3811.04,-4466.99,12.718,0,0,4.96409,0,0),
(54310,46,-3807.09,-4479.09,12.6775,1000,497902,5.08975,0,0);
DELETE FROM creature_linking WHERE guid = 54571;
INSERT INTO creature_linking (guid, master_guid, flag) VALUES
(54571, 54310, 512);
DELETE FROM dbscripts_on_creature_movement WHERE id BETWEEN 497901 AND 497902; 
INSERT INTO dbscripts_on_creature_movement (id, delay, command, datalong, datalong2, buddy_entry, search_radius, data_flags, dataint, dataint2, dataint3, dataint4, x, y, z, o, comments) VALUES
(497901,1,21,1,0,0,0,0,0,0,0,0,0,0,0,0,'Theramore Guard - active'),
(497902,1,21,0,0,0,0,0,0,0,0,0,0,0,0,0,'Theramore Guard - unactive');

-- Stitched Horror - correct spawn
UPDATE creature SET position_x = 3359.098633, position_y = -3048.040527, position_z = 165.255981, orientation = 2.147723 WHERE guid = 86295;
