
/*
 * This file definds generic things that should be defined
 * prior to defining a ruleset. 
 * 
 * NOTE: This file is not included from main.js.inc .
 * 
 */

// weapon path role
const ROLE = {
	AT: 0, 
	AP: 1, 
	AS: 2, 
	AA: 3,
	LENGTH: 4, // number of items in this enum
}

// indexed by the previous enum's values
const ROLENAME = [
	"AT",
	"AP",
	"AS",
	"AA",
];

// what sort of body to use for this weapon
const WEIGHT = {
	LIGHT: 0, 
	MEDIUM: 1, 
	HEAVY: 2, 
}

// what to use this defensive structure for
const DEFROLE = {
	STANDALONE: 0,
	GATEWAY: 1,
	ARTY: 2,
	ANTIAIR: 3,
	FORTRESS: 4,
}

// return values of build order calls
const BUILDRET = {
	SUCCESS: 0,
	UNAVAILABLE: 1,
	FAILURE: 2,
}

// should we execute a build call in an unsafe location?
const IMPORTANCE = {
	PEACETIME: 0,
	MANDATORY: 1,
}

// aspects of every research path
const RESASPECTS = {
	WEAPONS: 0,
	DEFENSES: 1,
	VTOLS: 2,
	EXTRAS: 3,
	LENGTH: 4, // number of items in this enum
}

// what to use this body for? (bit field)
const BODYUSAGE = {
	GROUND: 1, // for tanks
	AIR: 2, // for VTOLs
	COMBAT: 3, // GROUND | AIR
	TRUCK: 4, // trucks, sensors and repair droids
	UNIVERSAL: 7, // COMBAT | TRUCK
}

// what sort of weapons this body is resistant to?
const BODYCLASS = {
	KINETIC: 0,
	THERMAL: 1,
}

// should this module be prioritized when not having enough power?
const MODULECOST = {
	CHEAP: 0,
	EXPENSIVE: 1,
}
