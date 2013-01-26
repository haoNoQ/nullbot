
/*
 * This file is responsible for chat listening. It contains handler functions for different chat
 * messages; each such function takes message and player as a parameter and returns a reply string. 
 * All chat talking (such as calling for help) is coded in other places, not here.
 * 
 * Messages are marked as translatable when necessary. 
 * 
 */

(function(_global) {
////////////////////////////////////////////////////////////////////////////////////////////

_global.chatWho = function(args) {
	var str = "NullBot3 (" + scriptName + ") ";
	switch(difficulty) {
		case EASY: str += _("EASY"); break;
		case MEDIUM: str=str + _("MEDIUM"); break;
		case HARD: str=str + _("HARD"); break;
		case INSANE: str=str + _("INSANE"); break;
	}
	if (isAlly(args))
		str += (" ~" + personality.chatalias + "~");
	return str;
}

_global.chatSet = function(args) {
	var str = "";
	for (var i in subpersonalities) {
		if (subpersonalities[i].chatalias === args) {
			personality = subpersonalities[i];
			return _("Personality change successful.");
		}
		str = str + " " + subpersonalities[i].chatalias;
	}
	return _("No such personality! Try one of these:") + str;
}

_global.chatTruck = function(args) {
	var droid = enumTrucks().random();
	if (!defined(droid))
		return _("Sorry, I have no trucks.");
	if (donateObject(droid, args)) {
		addBeacon(droid.x, droid.y, args);
		return _("You can use this one.");
	} 
	return _("Sorry, droid transfer failed.");
}

_global.chatMoney = function(args) {
	var power = Math.round(myPower()/3);
	donatePower(power, args);
	return _("Power transfered.");
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
