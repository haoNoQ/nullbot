
/*
 * This file defines the scavenger AI personality.
 * 
 * It relies on ruleset definition in /rulesets/ to provide
 * scavenger strategy descriptions and necessary game stat information.
 * 
 * Then it passes control to the main code.
 * 
 */

// You can redefine these paths when you make a customized AI
// for a map or a challenge.
NB_PATH = "/multiplay/skirmish/";
NB_INCLUDES = NB_PATH + "nb_includes/";
NB_RULESETS = NB_PATH + "nb_rulesets/";

// please don't touch this line
include(NB_INCLUDES + "_head.js");

////////////////////////////////////////////////////////////////////////////////////////////
// Start the actual personality definition

// the rules in which this personality plays
include(NB_RULESETS + "scavfact.js");

// variables defining the personality
var subpersonalities = {
	SCAV: {
		chatalias: "ugh",
		weaponPaths: [
			weaponStats.scavengers,
		],
		earlyResearch: [
		],
		minTanks: 1, becomeHarder: 3, maxTanks: 16,
		minTrucks: 3, minHoverTrucks: 4, maxSensors: 1,
		minMiscTanks: 1, maxMiscTanks: 2,
		vtolness: 0, defensiveness: 0,
		maxPower: 0,
		repairAt: 0,
	},
};

function buildOrder() {
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Proceed with the main code

include(NB_INCLUDES + "_main.js");
