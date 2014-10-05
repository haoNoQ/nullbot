
/*
 * This file describes standard stats and strategies of
 * the scavenger AI.
 *
 */

// HACK: this should be there to make sure scavengers
// can produce stuff. With this we promise to never 
// implement eventGameInit in the includes.
function eventGameInit()
{
	makeComponentAvailable("B4body-sml-trike01", me);
	makeComponentAvailable("B3body-sml-buggy01", me);
	makeComponentAvailable("B2JeepBody", me);
	makeComponentAvailable("BusBody", me);
	makeComponentAvailable("FireBody", me);
	makeComponentAvailable("B1BaBaPerson01", me);
	makeComponentAvailable("BaBaProp", me);
	makeComponentAvailable("BaBaLegs", me);
	makeComponentAvailable("bTrikeMG", me);
	makeComponentAvailable("BuggyMG", me);
	makeComponentAvailable("BJeepMG", me);
	makeComponentAvailable("BusCannon", me);
	makeComponentAvailable("BabaFlame", me);
	makeComponentAvailable("BaBaMG", me);
	// also initialize startPositions[me], which always
	// remains uninitialized for the scavenger player
	var midX = mapWidth / 2, midY = mapHeight / 2;
	startPositions[me] = { x: midX, y: midY };
	var minDist = Infinity, minStr;
	var list = enumStruct(me, FACTORY);
	list.forEach(function(str, i) {
		var dist = distance(str, midX, midY);
		if (dist < minDist) {
			minDist = dist;
			minStr = str;
		}
	});
	if (defined(minStr)) {
		startPositions[me].x = minStr.x;
		startPositions[me].y = minStr.y;
	}
	baseLocation = startPositions[me];
}

const baseScale = 15;

const lassatSplash = 4;

// a special flag to choose templates randomly, instead of later=>better.
const randomTemplates = 1;

function iCanDesign() {
	return true;
}

const structures = {
	factories: [],
	templateFactories: [ "A0BaBaFactory", ],
	vtolFactories: [],
	labs: [],
	gens: [ "A0BaBaGenerator", ],
	hqs: [],
	vtolPads: [],
	derricks: [],
	extras: [],
	sensors: [],
};

const oilResources = [ "OilResource", ];

const powerUps = [ "OilDrum", "Crate" ];

const modules = [
];

const targets = [
	"A0LightFactory",
	"A0CyborgFactory",
	"A0VTolFactory1",
	"A0ResourceExtractor",
];

const miscTargets = [
	"A0ResourceExtractor",
];

const sensorTurrets = [
];

const fundamentalResearch = [
];

const fastestResearch = [
];

// body and propulsion arrays don't affect fixed template droids
const bodyStats = [
];

const classResearch = {
	kinetic: [
		[ // OBJTYPE.TANK
		],
		[ // OBJTYPE.BORG
		],
		[ // OBJTYPE.DEFS
		],
		[ // OBJTYPE.VTOL
		],
	],
	thermal: [
		[
		],
		[
		],
		[
		],
		[
		],
	],
}

// We still need those to figure out the map topology, even though
// scavengers don't use that.
const propulsionStats = [
	{ res: "R-Vehicle-Prop-Wheels", stat: "wheeled01", usage: PROPULSIONUSAGE.GROUND },
	{ res: "R-Vehicle-Prop-Halftracks", stat: "HalfTrack", usage: PROPULSIONUSAGE.GROUND },
	{ res: "R-Vehicle-Prop-Tracks", stat: "tracked01", usage: PROPULSIONUSAGE.GROUND },
	{ res: "R-Vehicle-Prop-Hover", stat: "hover01", usage: PROPULSIONUSAGE.HOVER },
	{ res: "R-Vehicle-Prop-VTOL", stat: "V-Tol", usage: PROPULSIONUSAGE.VTOL },
];

const truckTurrets = [
];

const truckTemplates = [
];

const fallbackWeapon = 'scavengers';

const weaponStats = {
	scavengers: {
		roles: [ 0.25, 0.25, 0.25, 0.25 ], chatalias: "uhm", micro: MICRO.MELEE, 
		extras: [], weapons: [], vtols: [], defenses: [], templates: [
			{ res: "", body: "B4body-sml-trike01", prop: "BaBaProp", weapons: [ "bTrikeMG", ] },
			{ res: "", body: "B3body-sml-buggy01", prop: "BaBaProp", weapons: [ "BuggyMG", ] },
			{ res: "", body: "B2JeepBody", prop: "BaBaProp", weapons: [ "BJeepMG", ] },
			{ res: "", body: "BusBody", prop: "BaBaProp", weapons: [ "BusCannon", ] },
			{ res: "", body: "FireBody", prop: "BaBaProp", weapons: [ "BabaFlame", ] },
		],
	},
};
