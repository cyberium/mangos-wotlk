-- Complete classlevelstats table for rogue class
UPDATE creature_template_classlevelstats AS t1,
(SELECT BaseDamageExp0, BaseDamageExp1, BaseDamageExp2, BaseMeleeAttackPower, BaseRangedAttackPower, level  FROM creature_template_classlevelstats WHERE Class = 2) as t2
SET
    t1.BaseDamageExp0        = t2.BaseDamageExp0,
    t1.BaseDamageExp1        = t2.BaseDamageExp1,
    t1.BaseDamageExp2        = t2.BaseDamageExp2,
    t1.BaseMeleeAttackPower  = t2.BaseMeleeAttackPower,
    t1.BaseRangedAttackPower = t2.BaseRangedAttackPower
WHERE (t1.level = t2.level AND t1.class = 4);