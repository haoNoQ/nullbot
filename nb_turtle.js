
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
include(NB_RULESETS + "standard.js.inc");

// variables defining the personality
var subpersonalities = {
	MR: {
		chatalias: "mr",
		weaponPaths: [
			weaponStats.rockets_AT,
			weaponStats.machineguns,
			weaponStats.rockets_AS,
			weaponStats.rockets_AA,
			weaponStats.rockets_late_AP,
		],
		earlyResearch: [
			"R-Defense-Tower01",
			"R-Defense-Pillbox01",
			"R-Defense-Tower06",
			"R-Defense-MRL",
		],
		minTanks: 3, becomeHarder: 3, maxTanks: 21,
		minTrucks: 5, minHoverTrucks: 4,
		minMiscTanks: 1, maxMiscTanks: 6,
		vtolness: 100,
		defensiveness: 100, // this enables turtle AI specific code
		maxPower: 300,
		repairAt: 50,
	},
	MC: {
		chatalias: "mc",
		weaponPaths: [
			weaponStats.cannons,
			weaponStats.machineguns,
			weaponStats.mortars,
			weaponStats.fireMortars,
			weaponStats.cannons_AA,
		],
		earlyResearch: [
			"R-Defense-Tower01",
			"R-Defense-Pillbox04",
			"R-Defense-WallTower02",
			"R-Defense-MortarPit",
		],
		minTanks: 3, becomeHarder: 3, maxTanks: 21,
		minTrucks: 5, minHoverTrucks: 4,
		minMiscTanks: 1, maxMiscTanks: 6,
		vtolness: 100, defensiveness: 100,
		maxPower: 300,
		repairAt: 50,
	},
};

// this function describes the early build order
// you can rely on personality.chatalias for choosing different build orders for
// different subpersonalities
function buildOrder() {
	var derrickCount = countFinishedStructList(structures.derricks);
	// might be good for Insane AI, or for rebuilding
	if (derrickCount > 0)
		if (buildMinimum(structures.gens, 1)) return true;
	// lab, factory, gen, cc - the current trivial build order for the 3.2+ starting conditions
	if (buildMinimum(structures.factories, 1)) return true;
	if (buildMinimum(structures.labs, 1)) return true;
	if (buildMinimum(structures.gens, 1)) return true;
	// make sure trucks go capture some oil at this moment
	if (buildMinimumDerricks(1)) return true;
	// what if one of them is being upgraded? will need the other anyway.
	// also, it looks like the right timing in most cases.
	if (buildMinimum(structures.gens, 2)) return true;
	if (buildMinimum(structures.hqs, 1)) return true;
	// make sure we have at least that much oils by now
	if (buildMinimumDerricks(5)) return true;
	// support hover maps
	if (buildMinimum(structures.labs, 4)) return true;
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Proceed with the main code

include(NB_INCLUDES + "_main.js.inc");
