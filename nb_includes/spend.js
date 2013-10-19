
/*
 * This file is responsible for the money spending system. It makes sure AI is not stuck 
 * at zero money for too long, and spends money on everything uniformly, considering
 * adaptive choices.
 * 
 */

////////////////////////////////////////////////////////////////////////////////////////////
(function(_global) {

_global.spendMoney = function() {
	queue("checkResearch", 100);
	queue("checkConstruction", 200);
	queue("checkProduction", 300);
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
