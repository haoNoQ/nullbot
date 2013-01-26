
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

_global.chatWho = function(player, args) {
	var str = "NullBot3 (" + scriptName + ") ";
	switch(difficulty) {
		case EASY: str += _("EASY"); break;
		case MEDIUM: str=str + _("MEDIUM"); break;
		case HARD: str=str + _("HARD"); break;
		case INSANE: str=str + _("INSANE"); break;
	}
	if (isAlly(player))
		str += (" ~" + personality.chatalias + "~");
	return str;
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
