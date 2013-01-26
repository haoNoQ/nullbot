
/*
 * This file includes event definitions only.
 * 
 */

function eventGameInit() {
	queue("setTimers", me * 100);
}

function eventStartLevel() {
	// initialize pseudo-random subpersonality here
	var j = 1, s = 0;
	for (var i = 0; i < maxPlayers; ++i) {
		if (allianceExistsBetween(me, i))
			s += j;
		j *= 2;
	}
	var s = s + (new Date()).getMinutes(); // the random "s" number obtained here is the same for all players in any team
	j = 0;
	for (var i in subpersonalities) // count the amount of subpersonalities
		++j;
	s = s % j;
	j = 0;
	for (var i in subpersonalities) {
		if (j === s)
			personality = subpersonalities[i];
		++j;
	}
	// the following code is necessary to avoid some strange game bug when droids that 
	// are initially buried into the ground fail to move out of the way when a building 
	// is being placed right above them
	enumTrucks().forEach(function(droid) {
		orderDroidLoc(droid, DORDER_MOVE, droid.x + random(3) - 1, droid.y + random(3) - 1);
	});
}

function eventDroidBuilt(droid, structure) {
	groupDroid(droid);
}

function eventStructureBuilt(structure) {
	queue("checkConstruction");
}

function eventDestroyed(object) {
	invalidateTarget(object);
	invalidateVtolTarget(object);
}

function eventAttacked(victim, attacker) {
	if (attacker === null)
		return; // no idea why it happens sometimes
	if (isAlly(attacker.player))
		return; // don't respond to accidental friendly fire
	if (victim.type === DROID) {
		if (!isVTOL(victim) && defined(victim.group)) {
			fallBack(victim, attacker);
			setTarget(attacker, victim.group);
			touchGroup(victim.group);
		}
	} else if (victim.type === STRUCTURE) {
		if (throttled(5000))
			return;
		if (inPanic()) {
			if (victim.player !== me)
				return;
			else
				for (var i = 0; i < MAX_GROUPS; ++i)
					if (groupSize(i) > 0)
						setTarget(attacker, i);
		}
		setTarget(attacker, miscGroup);
		setTarget(attacker);
	}
}

function eventStructureReady(structure) {
	fireLassat(structure);
}

function eventChat(from, to, message) {
	// we are not case-sensitive
	message = message.toLowerCase();
	if (message === "!nb who") {
		chat(from, chatWho(from)); 
		return;
	}
	// don't reply on any other message coming from enemies
	if (!isAlly(from))
		return;
	if (message.substr(0, 7) === "!nb set") {
		chat(from, chatSet(message.substr(8)));
		return;
	}
	if (message.substr(0, 7) === "!nb res") {
		chat(from, chatRes(message.substr(8)));
		return;
	}
	if (message === "!nb truck") {
		chat(from, chatTruck(from));
		return;
	}
	if (message === "!nb power" || message === "!nb money") {
		chat(from, chatMoney(from));
		return;
	}
}

function eventObjectTransfer(object, from) {
	if (object.player !== me)
		return; // object was transferred from me, not to me
	if (object.type === DROID)
		groupDroid(object);
}
