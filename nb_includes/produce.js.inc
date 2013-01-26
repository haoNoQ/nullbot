
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
		if (componentAvailable(list[i])) 
			return list;
	if (forVtol)
		return weaponStatsToVtolComponents(personality.weaponPaths.random());
	else
		return weaponStatsToComponents(personality.weaponPaths.random());
}

// HACK: this wrapper is necessary to use componentAvailable in functions like Array.filter
// because componentAvailable acts weirdly when it receives more than one argument
// (backward compatibility problems, see API documentation on componentAvailable);
function myComponentAvailable(comp) {
	return componentAvailable(comp);
}

function chooseBodyWeaponPair(bodies, weapons) {
	var success = false;
	var body, weapon;
	weapons.filter(myComponentAvailable).forEach(function(w) {
		if (success) 
			return;
		var ww = guessWeaponWeight(w);
		bodies.filter(myComponentAvailable).forEach(function(b) {
			if (success) 
				return;
			var bw = guessBodyWeight(b);
			switch(ww) {
				case WEIGHT.ULTRALIGHT: 
					if (bw <= WEIGHT.LIGHT)
						success = true;
					break;
				case WEIGHT.LIGHT:
					if (bw <= WEIGHT.MEDIUM)
						success = true;
					break;
				case WEIGHT.MEDIUM:
					success = true;
					break;
				case WEIGHT.HEAVY:
					if (bw >= WEIGHT.MEDIUM)
						success = true;
					break;
				case WEIGHT.ULTRAHEAVY:
					if (bw >= WEIGHT.HEAVY)
						success = true;
					break;
			}
			if (success) {
				body = b;
				weapon = w;
			}
		});
	});
	if (success)
		return {b: body, w: weapon};
}

function produceTank(factory) {
	var cls = chooseBodyClass();
	var bodies = [];
	if (cls === BODYCLASS.KINETIC) {
		bodies = bodies.concat(bodyStatsToComponents(bodyStats.kinetic, BODYUSAGE.GROUND));
		bodies = bodies.concat(bodyStatsToComponents(bodyStats.thermal, BODYUSAGE.GROUND));
	} else {
		bodies = bodies.concat(bodyStatsToComponents(bodyStats.thermal, BODYUSAGE.GROUND));
		bodies = bodies.concat(bodyStatsToComponents(bodyStats.kinetic, BODYUSAGE.GROUND));
	}
	var propulsions;
	var ret = scopeRatings();
	var rnd = random(ret.land + ret.sea);
	if (!defined(rnd)) // we need only vtols?
		return false;
	if (rnd < ret.land)
		propulsions = propulsionStatsToComponents(propulsionStats.ground);
	else 
		propulsions = propulsionStatsToComponents(propulsionStats.hover);
	var bwPair = chooseBodyWeaponPair(bodies, chooseWeapon());
	if (!defined(bwPair))
		return false;
	return ourBuildDroid(factory, "Tank", bwPair.b, propulsions, bwPair.w, bwPair.w, bwPair.w);
}

function produceVtol(factory) {
	// TODO: consider thermal bodies
	var bodies = bodyStatsToComponents(bodyStats.kinetic, BODYUSAGE.AIR);
	var propulsions = propulsionStatsToComponents(propulsionStats.vtol);
	var bwPair = chooseBodyWeaponPair(bodies, chooseWeapon(true));
	if (!defined(bwPair))
		return false;
	return ourBuildDroid(factory, "VTOL", bwPair.b, propulsions, bwPair.w, bwPair.w, bwPair.w);
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

function checkTruckProduction() {
	var trucks = enumTrucks();
	if (trucks.length < personality.minTrucks || (iHaveHover() && trucks.filter(function(droid) {
		return isHoverPropulsion(droid.propulsion);
	}).length < personality.minHoverTrucks))
		enumIdleStructList(structures.factories).forEach(function(factory) {
			produceTruck(factory);
		});
	if (enumTrucks().length < personality.minTrucks)
		enumIdleStructList(structures.templateFactories).forEach(function(factory) {
			produceTemplateFromList(factory, truckTemplates);
		});
}

function checkTankProduction() {
	if (!iCanDesign())
		return false; // don't cheat by producing tanks before design is available (also saves money for early generators)
	enumIdleStructList(structures.factories).forEach(function(factory) {
		return produceTank(factory);
	});
}

function checkTemplateProduction() {
	enumIdleStructList(structures.templateFactories).forEach(function(factory) {
		return produceTemplate(factory);
	});
}

function checkVtolProduction() {
	if (!iCanDesign())
		return false; // don't cheat by producing vtols before design is available
	enumIdleStructList(structures.vtolFactories).forEach(function(factory) {
		return produceVtol(factory);
	});
}

_global.checkProduction = function() {
	checkTruckProduction();
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
