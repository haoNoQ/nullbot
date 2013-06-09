
/*
 * This file defines a standard AI personality for the base game. 
 * 
 * It relies on ruleset definition in /rulesets/ to provide
 * standard strategy descriptions and necessary game stat information.
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
include(NB_INCLUDES + "_head.js.inc");

////////////////////////////////////////////////////////////////////////////////////////////
// Start the actual personality definition

// the rules in which this personality plays
include(NB_RULESETS + "mini.js.inc");

// variables defining the personality
var subpersonalities = {
	Main: {
		chatalias: "nvm",
		weaponPaths: [ // weapons to use; put late-game paths below!
			weaponStats.mg,
			weaponStats.aa,
			weaponStats.cn,
			weaponStats.fl,
			weaponStats.rx,
			weaponStats.mo,
		],
		earlyResearch: [ // fixed research path for the early game
			"R-MiniTrikeBody",
			"R-MiniMGBunker",
		],
		minTanks: 1, // minimal attack force at game start
		becomeHarder: 3, // how much to increase attack force every 5 minutes
		maxTanks: 16, // maximum for the minTanks value (since it grows at becomeHarder rate)
		minTrucks: 3, // minimal number of trucks around
		minHoverTrucks: 4, // minimal number of hover trucks around
		maxSensors: 1, // number of mobile sensor cars to produce
		minMiscTanks: 2, // number of tanks to start harassing enemy
		maxMiscTanks: 3, // number of tanks used for defense and harass
		vtolness: 65, // the chance % of not making droids when adaptation mechanism chooses vtols
		defensiveness: 10, // same thing for defenses; set this to 100 to enable turtle AI specific code
		maxPower: 800, // build expensive things if we have more than that
		repairAt: 60, // how much % healthy should droid be to join the attack group instead of repairing
	},
};

function buildOrder() {
	var derrickCount = countFinishedStructList(structures.derricks);
	// might be good for Insane AI, or for rebuilding
	if (derrickCount > 0) 
		if (buildMinimum(structures.gens, 1)) return true;
	if (buildMinimum(structures.labs, 1)) return true;
	if (buildMinimum(structures.templateFactories, 2)) return true;
	if (buildMinimum(structures.gens, 1)) return true;
	if (buildMinimumDerricks(2)) return true;
	if (buildMinimum(structures.gens, 2)) return true;
	if (buildMinimum(structures.hqs, 1)) return true;
	if (buildMinimum(structures.vtolFactories, 1)) return true;
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Proceed with the main code

include(NB_INCLUDES + "_main.js.inc");
