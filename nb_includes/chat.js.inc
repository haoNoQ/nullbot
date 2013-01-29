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

var prefix = '!nb';

// key: name in chat, value: function that will be executed
// function gets two params: sender and argument
var commandMap = {
    set: chatSet,
    res: chatRes,
    truck: chatTruck,
    power: chatMoney,
    money: chatMoney, // alias for "power"
};

_global.handleChatMessage = function(sender, receiver, message) {
	// don't reply on any other message coming sender enemies
	if (message ===  "!nb who") {
		chat(sender, chatWho(sender));
		return;
	}
	if (!isAlly(sender))
		return;
	var result = message.split(" ").filter(String);
	if (result[0] !== prefix)
		return;
	var command = result[1];
	var argument = result[2];
	if (defined(commandMap[command]))
		chat(sender, commandMap[command](sender, argument));
}

function chatWho(sender, argument) {
	var str = "NullBot3 (" + scriptName + ") ";
	switch(difficulty) {
		case EASY: str += _("EASY"); break;
		case MEDIUM: str=str + _("MEDIUM"); break;
		case HARD: str=str + _("HARD"); break;
		case INSANE: str=str + _("INSANE"); break;
	}
	if (isAlly(sender))
		str += (" ~" + personality.chatalias + "~");
	return str;
}

function chatSet(sender, argument) {
	var str = "";
	for (var i in subpersonalities) {
		if (subpersonalities[i].chatalias === argument) {
			personality = subpersonalities[i];
			return _("Personality change successful.");
		}
		str = str + " " + subpersonalities[i].chatalias;
	}
	return _("No such personality! Try one of these:") + str;
}

function chatRes(sender, argument) {
	if (argument === "cl") {
		setForcedResearch(); // clear
		return _("Forced research cleared, will research anything I want now.");
	}
	if (argument === "no") {
		setForcedResearch(null); // clear
		return _("Research blocked, will research nothing now.");
	}
	if (argument === "fn") {
		setForcedResearch(fundamentalResearch);
		return _("Researching fundamental technology.");
	}
	var str = " cl no fn";
	for (var i in weaponStats) {
		if (weaponStats[i].chatalias === argument) {
			setForcedResearch(weaponStatsToResList(weaponStats[i]));
			return _("Researching ") + weaponStats[i].chatalias;
		}
		if (weaponStats[i].chatalias.indexOf("useless") < 0)
			str = str + " " + weaponStats[i].chatalias;
	}
	return _("No such research path! Try one of these:") + str;
}

function chatTruck(sender, argument) {
	var droid = enumTrucks().random();
	if (!defined(droid))
		return _("Sorry, I have no trucks.");
	if (donateObject(droid, sender)) {
		addBeacon(droid.x, droid.y, sender);
		return _("You can use this one.");
	} 
	return _("Sorry, droid transfer failed.");
}

function chatMoney(sender, argument) {
	var power = Math.round(myPower()/3);
	donatePower(power, sender);
	return _("Power transferred.");
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
