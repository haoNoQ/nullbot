
/*
 * This file includes event definitions only.
 * 
 */

function eventGameInit() {
	queue("setTimers", me * 100);
}

function eventStartLevel() {
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
	queue("bringTrucksToBase");
}

function eventDestroyed(object) {
	invalidateTarget(object);
	invalidateVtolTarget(object);
	if (object.type === DROID && object.player == me && object.group != vtolGroup)
		touchGroup(object.group);
}

function eventAttacked(victim, attacker) {
	if (attacker === null)
		return; // no idea why it happens sometimes
	if (isAlly(attacker.player))
		return; // don't respond to accidental friendly fire
	if (victim.type === DROID) {
		setTarget(attacker, victim.group);
		if (!isVTOL(victim))
			fallBack(victim, attacker);
	} else if (victim.type === STRUCTURE) {
		if (inPanic()) {
			if (victim.player != me)
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
