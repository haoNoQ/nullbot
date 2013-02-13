
/*
 * This file controls the AI's research choices.
 * 
 */

(function(_global) {
////////////////////////////////////////////////////////////////////////////////////////////

var forcedResearch;

_global.setForcedResearch = function(list) {
	forcedResearch = list;
}

function doResearch(lab) {
	if (defined(forcedResearch)) {
		if (forcedResearch === null)
			return false;
		if (pursueResearch(lab, forcedResearch))
			return true;
	}
	// if we need to quickly get a certain propulsion to reach the enemy, prioritize that.
	var ret = scopeRatings();
	if (ret.land === 0 && ret.sea === 0 && !iHaveVtol())
		if (pursueResearch(lab, [
			propulsionStatsToResList(PROPULSIONUSAGE.VTOL),
			fastestResearch,
		].random())) 
			return true;
	if (ret.land === 0 && ret.sea !== 0 && !iHaveHover() && !iHaveVtol())
		if (pursueResearch(lab, [
			propulsionStatsToResList(PROPULSIONUSAGE.HOVER),
			propulsionStatsToResList(PROPULSIONUSAGE.VTOL),
			fastestResearch,
		].random()))
			return true;
	// otherwise, start with completing the fixed research path
	if (defined(personality.earlyResearch))
		if (pursueResearch(lab, personality.earlyResearch))
			return true;
	// then, see if we want to research some weapons
	var objType = chooseObjectType();
	if (withChance(70)) { // TODO: make a more thoughtful decision here
		var list = weaponStatsToResList(chooseAvailableWeaponPathByRoleRatings(personality.weaponPaths, chooseWeaponRole()), objType);
		if (pursueResearch(lab, list))
			return true;
	}
	if (withChance(65)) { // TODO: make a more thoughtful decision here
		if (chooseBodyClass() === BODYCLASS.KINETIC) {
			if (withChance(40)) {
				switch (objType) {
					case 2:
						if (pursueResearch(lab, classResearch.kinetic.defs))
							return true;
						break;
					case 1:
						if (pursueResearch(lab, classResearch.kinetic.borg))
							return true;
						break;
					case 0:
					case 3:
						if (pursueResearch(lab, classResearch.kinetic.tank))
							return true;
						break;
				}
			}
			if (objType === 0 || objType === 3 || (objType === 2 && personality.defensiveness < 100))
				if (pursueResearch(lab, bodyStatsToResList(BODYCLASS.KINETIC)))
					return true;
		} else {
			if (withChance(40))
				switch (objType) {
					case 2:
						if (pursueResearch(lab, classResearch.thermal.defs))
							return true;
						break;
					case 1:
						if (pursueResearch(lab, classResearch.thermal.borg))
							return true;
						break;
					case 0:
					case 3:
						if (pursueResearch(lab, classResearch.thermal.tank))
							return true;
						break;
				}
			if (objType === 0 || objType === 3 || (objType === 2 && personality.defensiveness < 100))
				if (pursueResearch(lab, bodyStatsToResList(BODYCLASS.THERMAL)))
					return true;
		}
	}
	// if nothing of the above holds, do some generic research
	if (pursueResearch(lab, fundamentalResearch))
		return true;
	return false;
	
}

_global.checkResearch = function() {
	enumIdleStructList(structures.labs).forEach(doResearch);
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
