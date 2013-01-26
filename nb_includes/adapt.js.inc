
/*
 * This file is responsible for the adaptation system. It gathers statistics about 
 * player choices and regulates the AI's choices accordingly.
 * 
 */

(function(_global) {
////////////////////////////////////////////////////////////////////////////////////////////

function adaptVote(our, their) {
	var l = our.length;
	var ourTotal = our.reduce(function(prev, curr) { return prev + curr; });
	var theirTotal = their.reduce(function(prev, curr) { return prev + curr; });
	if (theirTotal === 0)
		return random(l);
	var rating = [];
	for (var i = 0; i < l; ++i)
		rating[i] = their[i] / theirTotal;
	if (ourTotal > 0)
		for (var i = 0; i < l; ++i)
			rating[i] -= our[i] / ourTotal;
	var maxRating = -Infinity;
	var maxIdx = 0;
	for (var i = 0; i < l; ++i) 
		if (rating[i] > maxRating) {
			maxRating = rating[i];
			maxIdx = i;
		}
	return maxIdx;
}

//
// here be functions for gathering player statistics
//

function SimpleStat() {
	// propulsion classes used by the player
	this.tank = 0;
	this.borg = 0;
	this.defs = 0;
	this.vtol = 0,
	// weapon-propulsion classes used by the player (unused so far, due to API limitations)
	this.at = 0;
	this.ap = 0;
	this.as = 0;
	this.aa = 0;
	// armor used by the player
	this.kbody = 0;
	this.tbody = 0
	// weapon-armor classes used by the player (unused so far, due to API limitations)
	this.kweapon = 0;
	this.tweapon = 0;
}

function addStat(to, what, weight) {
	if (!defined(weight))
		weight = 1;
	for (var prop in to) 
		if (prop !== "addStat")
			to[prop] += what[prop] * weight;
}

const SCOPES = {
	land: 0,
	sea: 0,
	air: 0,
}

function ScopeStat() {
	// simple stats by scopes
	this.land = new SimpleStat();
	this.sea = new SimpleStat();
	this.air = new SimpleStat();
	// a function to convert this sort of stat to a SimpleStat
	this.collapse = function() {
		var ret = new SimpleStat();
		for (var i in SCOPES)
			addStat(ret, this[i]);
		return ret;
	}
}

function EnemyStat() {
	// separate stats for defense and offense
	this.defense = new ScopeStat();
	this.offense = new SimpleStat();
	// a function to convert this sort of stat to a SimpleStat
	this.collapse = function() {
		var ret = new SimpleStat();
		addStat(ret, this.defense.collapse());
		addStat(ret, this.offense);
		return ret;
	}
}

function MyStat() {
	// separate stats for defense and offense
	this.defense = new SimpleStat();
	this.offense = new ScopeStat();
	// a function to convert this sort of stat to a SimpleStat
	this.collapse = function() {
		var ret = new SimpleStat();
		addStat(ret, this.defense);
		addStat(ret, this.offense.collapse());
		return ret;
	}
}

var enemyStats = [];
for (var i = 0; i < maxPlayers; ++i)
	enemyStats[i] = new EnemyStat;

var myStats = new MyStat;

_global.enumLivingPlayers = function() {
	function uncached() {
		var ret = [];
		for (var i = 0; i < maxPlayers; ++i) {
			if (countStructList(targets, i) > 0) {
				ret.push(i);
				continue;
			}
			if (enumDroid(i).length > 0)
				ret.push(i);
		}
		return ret;
	};
	return cached(uncached, 30000);
}

_global.isAlly = function(player) {
	return allianceExistsBetween(me, player);
}

_global.isEnemy = function(player) {
	return !isAlly(player);
}

function canReachFromBase(propulsion, location) {
	if (defined(propulsion))
		return propulsionCanReach(propulsion, baseLocation.x, baseLocation.y, location.x, location.y);
}

function canReachBy(scope, location) {
	switch (scope) {
		case "land":
			return canReachFromBase(propulsionStatsToComponents(propulsionStats.ground).last(), location);
		case "sea":
			return canReachFromBase(propulsionStatsToComponents(propulsionStats.ground).last(), location);
		case "air":
			return canReachFromBase(propulsionStatsToComponents(propulsionStats.ground).last(), location);
	}
}

function threatensBase(droid) {
	if (isAlly(droid.player))
		return false;
	return droidCanReach(droid, baseLocation.x, baseLocation.y);
}

// count target structures reachable by land
function countLandTargets(player) {
	function uncached() {
		var currProp = propulsionStatsToComponents(propulsionStats.ground).last();
		if (!defined(currProp)) 
			return 0;
		var list = enumStructList(targets, player);
		return list.filter(function(struct) {
			return canReachFromBase(currProp, struct);
		}).length;
	}
	return cached(uncached, 5000, player);
}

// count target structures reachable by sea but not by land
function countSeaTargets(player) {
	function uncached() {
		var currProp = propulsionStatsToComponents(propulsionStats.hover).last();
		if (!defined(currProp)) 
			return 0;
		var prevProp = propulsionStatsToComponents(propulsionStats.ground).last();
		return enumStructList(targets, player).filter(function(struct) {
			return (!defined(prevProp) || !canReachFromBase(prevProp, struct)) && canReachFromBase(currProp, struct);
		}).length;
	}
	return cached(uncached, 5000, player);
}

// count target structures reachable by air but not by land or by sea
function countAirTargets(player) {
	function uncached() {
		var currProp = propulsionStatsToComponents(propulsionStats.vtol).last();
		if (!defined(currProp)) 
			return 0;
		var prevProp = []
			.concat(propulsionStatsToComponents(propulsionStats.ground))
			.concat(propulsionStatsToComponents(propulsionStats.hover))
			.last();
		return enumStructList(targets, player).filter(function(struct) {
			return (!defined(prevProp) || !canReachFromBase(prevProp, struct)) && canReachFromBase(currProp, struct);
		}).length;
	}
	return cached(uncached, 5000, player);
}

// TODO: this function needs to be updated once more object properties
// become available for the API to use.
function classifyObject(obj) {
	var ret = new SimpleStat();
	if (obj.type === STRUCTURE && obj.stattype !== DEFENSE)
		return ret;
	if (obj.type === DROID && obj.droidType !== DROID_WEAPON && obj.droidType !== DROID_CYBORG)
		return ret;
	if (obj.type === FEATURE)
		return ret;
	for (var i = 0; i < obj.weapons.length; ++i) {
		switch(guessWeaponRole(obj.weapons[i].name)) {
			case ROLE.AT:
				ret.at += 1;
				break;
			case ROLE.AP:
				ret.ap += 1;
				break;
			case ROLE.AS:
				ret.as += 1;
				break;
			case ROLE.AA:
				ret.aa += 1;
				break;
			default: 
				if (obj.canHitAir && obj.canHitGround) {
					ret.at += 1 / 4;
					ret.ap += 1 / 4;
					ret.as += 1 / 4;
					ret.aa += 1 / 4;
				} else if (obj.canHitAir && !obj.canHitGround) {
					ret.aa += 1;
				} else {
					ret.at += 1 / 3;
					ret.ap += 1 / 3;
					ret.as += 1 / 3;
				}
		}
	}
	for (var i = 0; i < obj.weapons.length; ++i) {
		if (getWeaponInfo(obj.weapons[i].name).impactClass === "KINETIC")
			ret.kweapon += 1;
		else
			ret.tweapon += 1;
	}
	if (obj.type === STRUCTURE || (obj.type === DROID && isVTOL(obj)))
		ret.tbody += 1;
	else if (obj.type === DROID && obj.droidType === DROID_CYBORG)
		ret.kbody += 1;
	else {
		if (bodyStatsToComponents(bodyStats.thermal).someProperty("stat", obj.body))
			ret.tbody += 1;
		else if (bodyStatsToComponents(bodyStats.kinetic).someProperty("stat", obj.body))
			ret.kbody += 1;
		else {
			ret.tbody += 1 / 2;
			ret.kbody += 1 / 2;
		}
	}
	if (obj.type === STRUCTURE)
		ret.defs += 1;
	if (obj.type === DROID) {
		if (isVTOL(obj))
			ret.vtol += 1;
		else if (obj.droidType === DROID_CYBORG)
			ret.borg += 1;
		else 
			ret.tank += 1;
	}
	return ret;
}

function summUpEnemyObject(obj) {
	var ret = classifyObject(obj);
	var p = obj.player;
	var w = obj.cost;
	if (obj.type === STRUCTURE) {
		for (var scope in SCOPES)
			if (canReachBy(scope, obj)) // structures don't move, so they are usually counted as defense
				addStat(enemyStats[p].defense[scope], ret, w);
		if (obj.range > baseScale * 2) // unless they're covering the whole map with their range
			addStat(enemyStats[p].offense, ret, w);
	}
	if (obj.type === DROID) {
		for (var scope in SCOPES) 
			if (canReachBy(scope, obj)) // if the droid can't reach your base, we count it as defense only
				addStat(enemyStats[p].defense[scope], ret, w);
		if (threatensBase(obj)) // otherwise count them as offense as well
			addStat(enemyStats[p].offense, ret, w);
	}
}

function summUpMyObject(obj, stat) {
	var ret = classifyObject(obj);
	var w = obj.cost;
	if (obj.type === STRUCTURE) {
		addStat(stat.defense, ret, w);
		if (obj.range > baseScale * 2) 
			for (var scope in SCOPES)
				addStat(stat.offense[scope], ret, w);
	}
	if (obj.type === DROID) {
		if (obj.group === miscGroup)
			addStat(stat.defense, ret, w);
		var list = enumLivingPlayers();
		list.forEach(function(p) {
			if (isEnemy(p)) {
				if (countLandTargets(p) > 0)
					addStat(stat.offense.land, ret, w / list.length);
				if (countSeaTargets(p) > 0)
					addStat(stat.offense.sea, ret, w / list.length);
				if (countAirTargets(p) > 0)
					addStat(stat.offense.air, ret, w / list.length);
			}
		});
	}
}

function updateEnemyStats(player) {
	enemyStats[player] = new EnemyStat();
	enumStruct(player, DEFENSE).forEach(summUpEnemyObject);
	enumDroid(player, DROID_WEAPON).forEach(summUpEnemyObject);
	enumDroid(player, DROID_CYBORG).forEach(summUpEnemyObject);
}

function updateMyInfo() {
	if (throttled(30000))
		return;
	myStats = new MyStat();
	enumStruct(me, DEFENSE).forEach(function(obj) { summUpMyObject(obj, myStats); });
	enumDroid(me, DROID_WEAPON).forEach(function(obj) { summUpMyObject(obj, myStats); });
	enumDroid(me, DROID_CYBORG).forEach(function(obj) { summUpMyObject(obj, myStats); });
}

_global.updateAllAdaptInfo = function() {
	var success = false;
	var enemies = enumLivingPlayers().filter(isEnemy);
	for (var p = 0; p < enemies.length; ++p) {
		if (!success && !throttled(3000 * (maxPlayers + 1), enemies[p])) { // need to make sure we update ourselves at least sometimes
			updateEnemyStats(enemies[p]);
			success = true;
		}
	}
	if (!success)
		updateMyInfo();
}

function getMyGroupInfo(gr) {
	var ret = new MyStat();
	enumGroup(gr).forEach(function(obj) { summUpMyObject(obj, ret); });
	return ret;
}

//
// here be functions for querying statistics gathered above
// and making adaptive decisions
//

function groupOurs(gr) {
	function uncached() {
		if (defined(gr))
			return getMyGroupInfo(gr).collapse();
		else 
			return myStats.collapse();
	}
	return cached(uncached, 500, gr);
}

function groupTheirs(gr) {
	function uncached() {
		if (defined(gr)) {
			if (gr === miscGroup) {
				var theirs = new SimpleStat();
				enumLivingPlayers().filter(isEnemy).forEach(function(p) {
					addStat(theirs, enemyStats[p].offense);
				});
				return theirs;
			} else 
				return enemyStats[gr].collapse();
		} else {
			var theirs = new SimpleStat();
			enumLivingPlayers().filter(isEnemy).forEach(function(p) {
				addStat(theirs, enemyStats[p].collapse());
			});
			return theirs;
		}
	}
	return cached(uncached, 500, gr);
}

_global.chooseWeaponRole = function(gr) {
	function uncached() {
		var our = groupOurs(gr), their = groupTheirs(gr);
		return adaptVote(
			[ our.at, our.ap, our.as, our.aa ],
			[ their.tank, their.borg, their.defs, their.vtol ]
		);
	}
	return cached(uncached, 5000, gr);
}

_global.chooseBodyClass = function(gr) {
	function uncached() {
		var our = groupOurs(gr), their = groupTheirs(gr);
		return adaptVote(
			[ our.kbody, our.tbody ],
			[ their.kweapon, their.tweapon ]
		);
	}
	return cached(uncached, 5000, gr);
}

_global.chooseObjectType = function() {
	function uncached() {
		var our = groupOurs(), their = groupTheirs();
		if (withChance(personality.defensiveness) && adaptVote(
				[ our.tank + our.borg + our.vtol, our.defs ],
				[ their.as, their.at + their.ap + their.aa ]
			) === 1)
			return 2;
		if (iHaveVtol() && adaptVote(
				[ our.tank + our.borg + our.defs, our.vtol ],
				[ their.aa, their.at + their.ap + their.as ]
			) === 1)
			return 3;
		return adaptVote(
			[ our.tank, our.borg ],
			[ their.ap, their.at ]
		);
	}
	return cached(uncached, 5000);
}

_global.scopeRatings = function() {
	var ret = { land: 0, sea: 0, air: 0 };
	enumLivingPlayers().filter(isEnemy).forEach(function(player) {
		ret.land += countLandTargets(player);
		ret.sea += countSeaTargets(player);
		ret.air += countAirTargets(player);
	});
	return ret;
}

_global.spendMoney = function() {
	queue("checkResearch", 100);
	queue("checkConstruction", 200);
	queue("checkProduction", 300);
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
