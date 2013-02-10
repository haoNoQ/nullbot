
/*
 * This file contain functions for manipulating stats defined by rulesets.
 * 
 */

(function(_global) {
////////////////////////////////////////////////////////////////////////////////////////////

_global.isHoverPropulsion = function(str) {
	return propulsionStats.hover.someProperty("stat", str);
}

_global.isVtolPropulsion = function(str) {
	return propulsionStats.vtol.someProperty("stat", str);
}

_global.iHaveHover = function() {
	return propulsionStats.hover.some(function(val ){return componentAvailable(val.stat)});
}

_global.iHaveVtol = function() {
    return propulsionStats.vtol.some(function(val ){return componentAvailable(val.stat)});
}

_global.iHaveArty = function() {
	for (var stat in weaponStats)
		for (var i = 0; i < weaponStats[stat].defenses.length; ++i)
			if (weaponStats[stat].defenses[i].defrole === DEFROLE.ARTY)
				if (countStruct(weaponStats[stat].defenses[i].stat) > 0)
					return true;
	return false;
}

// works with stored droid objects too!
_global.safeIsVtol = function(droid) {
	return isVtolPropulsion(droid.propulsion);
}

_global.enumStructList = function(list, player) {
	if (!defined(player))
		player = me;
	return list.reduce(function(summ, new_value) { return summ.concat(enumStruct(player, new_value)); }, []);
}

_global.countStructList = function(list, player) {
	if (!defined(player))
		player = me;
	return list.reduce(function(summ, new_value) { return summ + countStruct(new_value, player); }, 0);
}

_global.enumFinishedStructList = function(list, player) {
	return enumStructList(list, player).filterProperty("status", BUILT);
}

_global.countFinishedStructList = function(list, player) {
	return enumFinishedStructList(list, player).length;
}

_global.enumIdleStructList = function(list, player) {
	return enumFinishedStructList(list, player).filter(structureIdle);
}

_global.structListLimit = function(list) {
	return list.reduce(function (summ, val) {return summ + getStructureLimit(val)}, 0)
}

_global.guessWeaponRole = function (name) {
	for (var stat in weaponStats) {
		if (
			weaponStats[stat].weapons.someProperty("stat", name) ||
			weaponStats[stat].vtols.someProperty("stat", name) ||
			weaponStats[stat].templates.some(function (i) {return i.weapons.indexOf(name) > -1;})
		)
		return weaponStats[stat].role;
	}
	niceDebug("Ruleset warning: Couldn't guess weapon role of", name);
}

function guessWeaponMicro(name) {
	function uncached() {
		for (var stat in weaponStats) {
			if (weaponStats[stat].weapons.someProperty("stat", name))
				return weaponStats[stat].micro;
			if (weaponStats[stat].templates.some(function(i) { return i.weapons.indexOf(name) > -1; }))
				return weaponStats[stat].micro;
		}
		niceDebug("Ruleset warning: Couldn't guess weapon micro of", name);
	}
	return cached(uncached, Infinity, name);
}

_global.guessDroidMicro = function(droid) {
	for (var i = 0; i < droid.weapons.length; ++i) {
		var ret = guessWeaponMicro(droid.weapons[i].name);
		if (ret !== MICRO.RANGED)
			return ret;
	}
	return MICRO.RANGED;
}

function guessWeaponMicro(name) {
	function uncached() {
		for (var stat in weaponStats)
			if (weaponStats[stat].weapons.someProperty("stat", name))
				return weaponStats[stat].micro;
			if (weaponStats[stat].templates.some(function(i) { return i.weapons.indexOf(name) > -1; }))
				return weaponStats[stat].micro;
	}
	return cached(uncached, Infinity, name);
}

_global.guessDroidMicro = function(droid) {
	for (var i = 0; i < droid.weapons.length; ++i) {
		var ret = guessWeaponMicro(droid.weapons[i].name);
		if (ret !== MICRO.RANGED)
			return ret;
	}
	return MICRO.RANGED;
}

_global.guessBodyClass = function(name) {
	if (bodyStats.someProperty("stat", name))
		return BODYCLASS.THERMAL;
	if (bodyStats.someProperty("stat", name))
		return BODYCLASS.KINETIC;
	niceDebug("Ruleset warning: Couldn't guess body class of", name);
}

//
// here be functions for querying out research paths 
//

function statsToResList(path) {
	return path.map(function (val) {return val.res})
}

// todo make one StatsToResList and do filtering for path outside
_global.bodyStatsToResList = function(armor) {
    return statsToResList(filterBodyStatsByUsage(armor));
}

_global.propulsionStatsToResList = function(path) {
	return statsToResList(path);
}

_global.weaponStatsToResList = function(path, aspect) {
	var ret = [];
	if (!defined(aspect) || aspect === RESASPECTS.WEAPONS) {
		ret.concat(statsToResList(path.weapons), statsToResList(path.vtols), statsToResList(path.templates))
	}

    if (!defined(aspect) || aspect === RESASPECTS.DEFENSES) {
        ret.concat(statsToResList(path.defenses))
	}
	if (!defined(aspect) || aspect === RESASPECTS.EXTRAS)
		ret = ret.concat(path.extras);
	return ret;
}

//
// here be functions for querying out component lists
//

// todo move this to math. If we have same structure of data <list of objects> we can use it all around.
_global.filterDataByFlag = function (data, attr_name, flag) {
     return data.filter(function(obj) { return obj[attr_name] & flag; });
}


_global.filterBodyStatsByUsage = function(usage, armor) {
    if (defined(armor))
        {data = filterDataByFlag(bodyStats, 'armor', armor)}
    else
        {data = bodyStats}
    return filterDataByFlag(data, 'usage', usage).reverse();
}


_global.propulsionStatsToComponents = function(stats) {
	var ret = [];
	for (var i = 0; i < stats.length; ++i)
		ret.push(stats[i].stat);
	ret.reverse();
	return ret;
}

//
// here be functions for querying out defensive structures
//

_global.weaponStatsToDefenses = function(stats, defrole) {
	var ret = [];
	for (var i = 0; i < stats.defenses.length; ++i)
		if (!defined(defrole) || stats.defenses[i].defrole === defrole)
			ret.push(stats.defenses[i].stat);
	// reverse not needed here
	return ret;
}

_global.weaponStatsListToAvailableDefenses = function(list, defrole) {
	return list.reduce(function(one, two) { return one.concat(weaponStatsToDefenses(two, defrole).filter(isAvailable)); }, []);
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
