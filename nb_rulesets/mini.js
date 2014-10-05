
/*
 * This file describes standard stats and strategies of
 * the scavenger AI.
 *
 */

const randomTemplates = false;

const baseScale = 15;

const lassatSplash = 4;

function iCanDesign() {
	return true;
}

const structures = {
	factories: [],
	templateFactories: [ "MiniLightFactory", "MiniHeavyFactory", ],
	vtolFactories: [ "MiniChopperFactory", ],
	labs: [ "MiniLaboratory", "MiniAdvancedLaboratory", ],
	gens: [ "MiniPowerGenerator", ],
	hqs: [ "MiniCC", ],
	vtolPads: [ "MiniRearmPad", ],
	derricks: [ "A0ResourceExtractor", ],
	extras: [ "MiniAdvancedLaboratory", "MiniHeavyFactory", ],
	sensors: [ "MiniRepairFacility" ],
};

const oilResources = [ "OilResource", ];

const powerUps = [ "OilDrum", "Crate" ];

const modules = [];

const targets = [
	"MiniLightFactory",
	"MiniHeavyFactory",
	"MiniChopperFactory",
	"A0ResourceExtractor",
];

const miscTargets = [
	"A0ResourceExtractor",
];

const sensorTurrets = [
];

const fundamentalResearch = [
	"R-MiniRepairFacility",
	"R-MiniRearmPad",
];

const fastestResearch = [
	"R-MiniAdvancedLaboratory",
	"R-MiniRearmPad",
];

const bodyStats = [
	{ res: "R-MiniTrikeBody", stat: "MiniTrikeBody", weight: WEIGHT.LIGHT, usage: BODYUSAGE.UNIVERSAL, armor: BODYCLASS.KINETIC },
	{ res: "R-MiniJeepBody", stat: "MiniJeepBody", weight: WEIGHT.LIGHT, usage: BODYUSAGE.UNIVERSAL, armor: BODYCLASS.KINETIC },
	{ res: "R-MiniPoliceChopperBody", stat: "MiniPoliceChopperBody", weight: WEIGHT.LIGHT, usage: BODYUSAGE.UNIVERSAL, armor: BODYCLASS.THERMAL },
	{ res: "R-MiniCamperBody", stat: "MiniCamperBody", weight: WEIGHT.LIGHT, usage: BODYUSAGE.UNIVERSAL, armor: BODYCLASS.KINETIC },
	{ res: "R-MiniSchoolBusBody", stat: "MiniSchoolBusBody", weight: WEIGHT.LIGHT, usage: BODYUSAGE.UNIVERSAL, armor: BODYCLASS.KINETIC },
	{ res: "R-MiniTractorBody", stat: "MiniTractorBody", weight: WEIGHT.LIGHT, usage: BODYUSAGE.UNIVERSAL, armor: BODYCLASS.KINETIC },
	{ res: "R-MiniIcevanBody", stat: "MiniIcevanBody", weight: WEIGHT.LIGHT, usage: BODYUSAGE.UNIVERSAL, armor: BODYCLASS.THERMAL },
	{ res: "R-MiniBuggyBody", stat: "MiniBuggyBody", weight: WEIGHT.LIGHT, usage: BODYUSAGE.UNIVERSAL, armor: BODYCLASS.KINETIC },
	{ res: "R-MiniFiretruckBody", stat: "MiniFiretruckBody", weight: WEIGHT.LIGHT, usage: BODYUSAGE.UNIVERSAL, armor: BODYCLASS.THERMAL },
	{ res: "R-MiniHeavyTruckBody", stat: "MiniHeavyTruckBody", weight: WEIGHT.LIGHT, usage: BODYUSAGE.UNIVERSAL, armor: BODYCLASS.KINETIC },
	{ res: "R-MiniPoliceChopperBody", stat: "MiniPoliceChopperBody", weight: WEIGHT.LIGHT, usage: BODYUSAGE.UNIVERSAL, armor: BODYCLASS.KINETIC },
];

const classResearch = {
	kinetic: [
		[ // OBJTYPE.TANK
			"R-MiniJeepBody",
			"R-MiniCamperBody",
			"R-MiniSchoolBusBody",
		],
		[ // OBJTYPE.BORG
			"R-MiniJeepBody",
			"R-MiniCamperBody",
			"R-MiniSchoolBusBody",
		],
		[ // OBJTYPE.DEFS
			"R-MiniMGBunker",
		],
		[ // OBJTYPE.VTOL
			"R-MiniPoliceChopperBody",
		],
	],
	thermal: [
		[
			"R-MiniIcevanBody",
			"R-MiniFiretruckBody",
		],
		[
			"R-MiniIcevanBody",
			"R-MiniFiretruckBody",
		],
		[
			"R-MiniCannonTower",
		],
		[
			"R-MiniMGChopperBody",
		],
	],
}

// we still need hover propulsion listed to understand map topology
const propulsionStats = [
	{ res: "R-Vehicle-Prop-Wheels", stat: "wheeled01", usage: PROPULSIONUSAGE.GROUND },
	{ res: "R-Vehicle-Prop-Hover", stat: "hover01", usage: PROPULSIONUSAGE.HOVER },
	{ res: "R-Vehicle-Prop-VTOL", stat: "V-Tol", usage: PROPULSIONUSAGE.VTOL },
];

const truckTurrets = [
];

const truckTemplates = [
	{ body: "Body1REC", prop: "wheeled01", weapons: [ "Spade1Mk1", ] },
	{ body: "MiniExcavatorBody", prop: "wheeled01", weapons: [ "MiniExcavatorArm", ] },
];

const fallbackWeapon = 'mg';

const weaponStats = {
	mg: {
		roles: [ 0.3, 0.3, 0.2, 0.2 ], chatalias: "mg", micro: MICRO.RANGED,
		weapons: [], vtols: [],
		defenses: [
			{ res: "R-MiniMGBunker", stat: "MiniMGBunker", defrole: DEFROLE.STANDALONE },
		],
		templates: [
			{ res: "R-MiniTrikeBody", body: "MiniTrikeBody", prop: "wheeled01", weapons: [ "MiniMG", ] },
			{ res: "R-MiniJeepBody", body: "MiniJeepBody", prop: "wheeled01", weapons: [ "MiniMG", ] },
			{ res: "R-MiniPoliceChopperBody", body: "MiniPoliceChopperBody", prop: "V-Tol", weapons: [ "MiniMGVTOL", "MiniMGVTOL", "MiniMGVTOL" ] },
		],
		extras: [
			"R-MiniMGUpgrade",
		]
	},
	aa: {
		roles: [ 0.0, 0.0, 0.0, 1.0 ], chatalias: "aa", micro: MICRO.RANGED,
		weapons: [], vtols: [],
		defenses: [
			{ res: "R-MiniZenithTower", stat: "MiniZenithTower", defrole: DEFROLE.STANDALONE },
		],
		templates: [], extras: []
	},
	cn: {
		roles: [ 0.25, 0.25, 0.50, 0 ], chatalias: "cn", micro: MICRO.RANGED,
		weapons: [], vtols: [],
		defenses: [
			{ res: "R-MiniCannonTower", stat: "MiniCannonTower", defrole: DEFROLE.STANDALONE },
		],
		templates: [
			{ res: "R-MiniCamperBody", body: "MiniCamperBody", prop: "wheeled01", weapons: [ "MiniCannon" ] },
			{ res: "R-MiniSchoolBusBody", body: "MiniSchoolBusBody", prop: "wheeled01", weapons: [ "MiniCannon", "MiniMG" ] },
		],
		extras: [
			"R-MiniCannonUpgrade",
		]
	},
	fl: {
		roles: [ 0.5, 0.5, 0.0, 0.0 ], chatalias: "fl", micro: MICRO.MELEE,
		weapons: [], vtols: [],
		defenses: [
			{ res: "R-MiniFlamerTower", stat: "MiniFlamerTower", defrole: DEFROLE.GATEWAY },
		],
		templates: [
			{ res: "R-MiniTractorBody", body: "MiniTractorBody", prop: "wheeled01", weapons: [ "MiniFlamer" ] },
			{ res: "R-MiniIcevanBody", body: "MiniIcevanBody", prop: "wheeled01", weapons: [ "MiniFlamer" ] },
		],
		extras: [
			"R-MiniFlamerUpgrade",
		],
	},
	rx: {
		roles: [ 0.3, 0.3, 0.4, 0.0 ], chataliases: "rx", micro: MICRO.RANGED,
		weapons: [], vtols: [],
		defenses: [
			{ res: "R-MiniRocketPit", stat: "MiniRocketPit", defrole: DEFROLE.STANDALONE },
			{ res: "R-MiniLancerPit", stat: "MiniLancerPit", defrole: DEFROLE.STANDALONE },
		],
		templates: [
			{ res: "R-MiniBuggyBody", body: "MiniBuggyBody", prop: "wheeled01", weapons: [ "MiniMRA" ] },
			{ res: "R-MiniFiretruckBody", body: "MiniFiretruckBody", prop: "wheeled01", weapons: [ "MiniLancer", "MiniMG" ] },
			{ res: "R-MiniHeavyTruckBody", body: "MiniHeavyTruckBody", prop: "wheeled01", weapons: [ "MiniMG", "MiniLancer", "MiniMRA" ] },
			{ res: "R-MiniPoliceChopperBody", body: "MiniPoliceChopperBody", prop: "V-Tol", weapons: [ "MiniPodVTOL" ] },
		],
		extras: [
			"R-MiniRocketUpgrade",
		]
	},
	mo: {
		roles: [ 0.1, 0.1, 0.8, 0.0 ], chataliases: "mo", micro: MICRO.DUMB,
		weapons: [], vtols: [],
		defenses: [
			{ res: "R-MiniMortarPit", stat: "MiniMortarPit", defrole: DEFROLE.ARTY },
		],
		templates: [], extras: []
	},
};
