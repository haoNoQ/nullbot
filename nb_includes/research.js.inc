
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
			propulsionStatsToResList(propulsionStats.vtol),
			fastestResearch,
		].random())) 
			return true;
	if (ret.land === 0 && ret.sea !== 0 && !iHaveHover() && !iHaveVtol())
		if (pursueResearch(lab, [
			propulsionStatsToResList(propulsionStats.hover),
			propulsionStatsToResList(propulsionStats.vtol),
			fastestResearch,
		].random()))
			return true;
	// otherwise, start with completing the fixed research path
	if (defined(personality.earlyResearch))
		if (pursueResearch(lab, personality.earlyResearch))
			return true;
	// then, see if we want to research some weapons
	if (withChance(75)) { // TODO: make a more thoughtful decision here
		var weaponPaths = personality.weaponPaths.filterProperty("role", chooseWeaponRole());
		var list = [];
		var aspect = random(5); // TODO: make a more thoughtful desicion here
		if (aspect > 3) aspect = 0;
		weaponPaths.forEach(function(statlist) {
			list = list.concat(weaponStatsToResList(statlist, aspect));
		});
		if (pursueResearch(lab, list))
			return true;
	}
	if (withChance(50)) { // TODO: make a more thoughtful decision here
		if (chooseBodyClass() === BODYCLASS.KINETIC) {
			if (withChance(50)) {
				switch (chooseObjectType()) {
					case 2:
						if (pursueResearch(lab, classResearch.kinetic.defs))
							return true;
					case 1:
						if (pursueResearch(lab, classResearch.kinetic.borg))
							return true;
					case 0:
						if (pursueResearch(lab, classResearch.kinetic.tank))
							return true;
				}
			}
			if (pursueResearch(lab, bodyStatsToResList(bodyStats.kinetic)))
				return true;
		} else {
			if (withChance(50))
				switch (chooseObjectType()) {
					case 2:
						if (pursueResearch(lab, classResearch.thermal.defs))
							return true;
					case 1:
						if (pursueResearch(lab, classResearch.thermal.borg))
							return true;
					case 0:
						if (pursueResearch(lab, classResearch.thermal.tank))
							return true;
				}
			if (pursueResearch(lab, bodyStatsToResList(bodyStats.thermal)))
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
