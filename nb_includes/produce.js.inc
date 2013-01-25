
/*
 * This file is responsible for droid production.
 * 
 */

(function(_global) {
////////////////////////////////////////////////////////////////////////////////////////////

function ourBuildDroid(factory, name, bodies, propulsions, weapons1, weapons2, weapons3) {
	buildDroid(factory, name, bodies, propulsions, "", "", weapons1, weapons2, weapons3);
}

function produceTruck(factory) {
	var bodies = bodyStatsToComponents(bodyStats.kinetic, BODYUSAGE.TRUCK);
	var propulsions = []
		.concat(propulsionStatsToComponents(propulsionStats.hover))
		.concat(propulsionStatsToComponents(propulsionStats.ground));
	return ourBuildDroid(factory, "Fancy Truck", bodies, propulsions, truckTurrets);
}

function weaponAvailable(weapon) {
	const COMP_WEAPON = 8;
	return componentAvailable(COMP_WEAPON, weapon);
}

// TODO: remove this "name" thing, which is only used by adapt code
// until droid weapons are exposed to API
function chooseWeapon(forVtol) {
	if (!defined(forVtol))
		forVtol = false;
	var role = chooseWeaponRole();
	var weaponPaths = personality.weaponPaths.filterProperty("role", role);
	var list = [];
	weaponPaths.forEach(function(statlist) {
		if (forVtol)
			list = list.concat(weaponStatsToVtolComponents(statlist));
		else
			list = list.concat(weaponStatsToComponents(statlist));
	});
	for (var i = 0; i < list.length; ++i)
		if (weaponAvailable(list[i])) 
			return { list: list, name: ROLENAME[role] };
	return { list: weaponStatsToComponents(personality.weaponPaths.random()), name: "" };
}

function produceTank(factory) {
	// TODO: consider thermal bodies
	var cls = chooseBodyClass();
	var bodies;
	if (cls === BODYCLASS.KINETIC)
		bodies = bodyStatsToComponents(bodyStats.kinetic, BODYUSAGE.GROUND);
	else
		bodies = bodyStatsToComponents(bodyStats.thermal, BODYUSAGE.GROUND);
	var propulsions;
	var ret = scopeRatings();
	var rnd = random(ret.land + ret.sea);
	if (!defined(rnd)) // we need only vtols?
		return false;
	if (rnd < ret.land)
		propulsions = propulsionStatsToComponents(propulsionStats.ground);
	else 
		propulsions = propulsionStatsToComponents(propulsionStats.hover);
	var w = chooseWeapon();
	return ourBuildDroid(factory, "Ground Tank " + w.name, bodies, propulsions, w.list, w.list, w.list);
}

function produceVtol(factory) {
	// TODO: consider thermal bodies
	var bodies = bodyStatsToComponents(bodyStats.kinetic, BODYUSAGE.AIR);
	var propulsions = propulsionStatsToComponents(propulsionStats.vtol);
	var w1 = chooseWeapon(true), w2 = chooseWeapon(true), w3 = chooseWeapon(true);
	return ourBuildDroid(factory, "VTOL " + w1.name, bodies, propulsions, w1.list, w2.list, w3.list);
}

function produceTemplateFromList(factory, list) {
	var ret = scopeRatings();
	for (var i = 0; i < list.length; ++i) {
		if (ret.land === 0 && !isHoverPropulsion(list[i].prop) && !isVtolPropulsion(list[i].prop))
			continue;
		if (ret.land === 0 && ret.sea === 0 && !isVtolPropulsion(list[i].prop))
			continue;
		if (ourBuildDroid(factory, "Template Droid", list[i].body, list[i].prop, list[i].weapons[0], list[i].weapons[1], list[i].weapons[2]))
			return true;
	}
	return false;
}

function produceTemplate(factory) {
	var list = [];
	if (enumTrucks().length < personality.minTrucks) 
		return produceTemplateFromList(factory, truckTemplates);
	else {
		var weaponPaths = personality.weaponPaths.filterProperty("role", chooseWeaponRole());
		weaponPaths.forEach(function(statlist) {
			var ret = statlist.templates;
			ret.reverse();
			list = list.concat(ret);
		});
		if (produceTemplateFromList(factory, list))
			return;
		// if didn't find a template of correct class, still produce templates;
		// TODO: it shouldn't work when some weapon paths become outdated
		return produceTemplateFromList(factory, personality.weaponPaths.random().templates);
	}
	return false;
}

function checkTankProduction() {
	enumIdleStructList(structures.factories).forEach(function(factory) {
		var trucks = enumTrucks();
		if (trucks.length < personality.minTrucks || (iHaveHover() && trucks.filter(function(droid) {
			return isHoverPropulsion(droid.propulsion);
		}).length < personality.minHoverTrucks))
			return produceTruck(factory);
		else {
			if (!iCanDesign())
				return false; // don't cheat by producing tanks before design is available (also saves money for early generators)
			return produceTank(factory);
		}
	});
}

function checkTemplateProduction() {
	enumIdleStructList(structures.templateFactories).forEach(function(factory) {
		return produceTemplate(factory);
	});
}

function checkVtolProduction() {
	enumIdleStructList(structures.vtolFactories).forEach(function(factory) {
		if (!iCanDesign())
			return false; // don't cheat by producing vtols before design is available
		return produceVtol(factory);
	});
}

_global.checkProduction = function() {
	switch(chooseObjectType()) {
		case 1:
			if (checkTemplateProduction())
				break;
		case 0:
			if (checkTankProduction())
				break;
		case 3:
			if (checkVtolProduction())
				break;
	}
	// if having too much energy, don't care about what we produce
	if (myPower() > personality.maxPower) {
		queue("checkConstruction");
		checkTemplateProduction();
		checkTankProduction();
		checkVtolProduction();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
