
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
	for (var i = 0; i < propulsionStats.hover.length; ++i)
		if (componentAvailable(propulsionStats.hover[i].stat))
			return true;
	return false;
}

_global.iHaveVtol = function() {
	for (var i = 0; i < propulsionStats.vtol.length; ++i)
		if (componentAvailable(propulsionStats.vtol[i].stat))
			return true;
	return false;
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

function enumFinishedStruct(player, stat) {
	return enumStruct(player, stat).filterProperty("status", BUILT);
}

function countFinishedStruct(player, stat) {
	return enumFinishedStruct(player, stat).length;
}

_global.enumFinishedStructList = function(list, player) {
	if (!defined(player))
		player = me;
	return list.reduce(function(summ, new_value) { return summ.concat(enumFinishedStruct(player, new_value)); }, []);
}

_global.countFinishedStructList = function(list, player) {
	if (!defined(player))
		player = me;
	return list.reduce(function(summ, new_value) { return summ + countFinishedStruct(new_value, player); }, 0);
}

_global.structListLimit = function(list) {
	var ret = 0;
	for (var i = 0; i < list.length; ++i)
		ret += getStructureLimit(list[i]);
	return ret;
}

_global.guessWeaponRole = function(name) {
	for (var stat in weaponStats) {
		if (weaponStats[stat].weapons.someProperty("stat", name))
			return weaponStats[stat].role;
		if (weaponStats[stat].vtols.someProperty("stat", name))
			return weaponStats[stat].role;
		if (weaponStats[stat].templates.some(function(i) { return i.weapons.indexOf(name) > -1; }))
			return weaponStats[stat].role;
	}
}

//
// here be functions for querying out research paths 
//

function statsToResList(path) {
	var ret = [];
	for (var i = 0; i < path.length; ++i) 
		ret.push(path[i].res);
	return ret;
}

_global.bodyStatsToResList = function(path) {
	return statsToResList(path);
}

_global.propulsionStatsToResList = function(path) {
	return statsToResList(path);
}

_global.weaponStatsToResList = function(path, aspect) {
	var ret = [];
	if (!defined(aspect) || aspect === RESASPECTS.WEAPONS) {
		for (var i = 0; i < path.weapons.length; ++i)
			ret.push(path.weapons[i].res);
		for (var i = 0; i < path.vtols.length; ++i)
			ret.push(path.vtols[i].res);
		for (var i = 0; i < path.templates.length; ++i)
			ret.push(path.templates[i].res);
	}
	if (!defined(aspect) || aspect === RESASPECTS.DEFENSES) {
		for (var i = 0; i < path.defenses.length; ++i)
			ret.push(path.defenses[i].res);
	}
	if (!defined(aspect) || aspect === RESASPECTS.EXTRAS)
		ret = ret.concat(path.extras);
	return ret;
}

//
// here be functions for querying out component lists
//

_global.bodyStatsToComponents = function(stats, usage, weight) {
	var ret = [];
	for (var i = 0; i < stats.length; ++i)
		if (!defined(weight) || stats[i].weight === weight)
			if (stats[i].usage & usage)
				ret.push(stats[i].stat);
	ret.reverse();
	return ret;
}

_global.propulsionStatsToComponents = function(stats) {
	var ret = [];
	for (var i = 0; i < stats.length; ++i)
		ret.push(stats[i].stat);
	ret.reverse();
	return ret;
}

_global.weaponStatsToComponents = function(stats, weight) {
	var ret = [];
	for (var i = 0; i < stats.weapons.length; ++i) {
		if (!defined(weight) || stats.weapons[i].weight === weight)
			ret.push(stats.weapons[i].stat);
	}
	ret.reverse();
	return ret;
}

_global.weaponStatsToVtolComponents = function(stats, weight) {
	var ret = [];
	for (var i = 0; i < stats.vtols.length; ++i) {
		if (!defined(weight) || stats.vtols[i].weight === weight)
			ret.push(stats.vtols[i].stat);
	}
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


////////////////////////////////////////////////////////////////////////////////////////////
})(this);
