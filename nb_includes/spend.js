
/*
 * This file is responsible for the money spending system. It makes sure AI is not stuck 
 * at zero money for too long, and spends money on everything uniformly, considering
 * adaptive choices.
 * 
 */

////////////////////////////////////////////////////////////////////////////////////////////
(function(_global) {

_global.fundamentalSpender = function() {
	return checkConstruction();
}

_global.researchSpender = function() {
	return checkResearch();
}

_global.combatSpender = function() {
	checkProduction();
	return true;
}

_global.spendMoney = function() {
	// do we have any money?
	if (myPower() < 0) return;
	if (fundamentalSpender()) return;
	if (researchSpender()) return;
	if (combatSpender()) return;
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
