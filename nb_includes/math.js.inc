
/*
 * This file includes routines and algorithms completely unrelated to Warzone 2100.
 * 
 */

(function(_global) {
////////////////////////////////////////////////////////////////////////////////////////////

// return random true or false in certain chance
// 0 with chance 0% will return true
// 50 with chance ~50% will return true
// 100 with chance 100% will return true
_global.withChance = function distance(chancePersent) {
	return random(100) <= chancePersent
}

//    Get distance between two points
//    acceptable arguments:
//    distance(obj, obj)
//    distance(x,y, obj)
//    distance(obj,x,y)
_global.distance = function distance(obj1, obj2, obj3, obj4) {
	var x1, x2, y1, y2;
	if (defined(obj1.x)) {
		x1 = obj1.x;
		y1 = obj1.y;
		if (defined(obj2.x)) {
			x2 = obj2.x;
			y2 = obj2.y;
		} else {
			x2 = obj2;
			y2 = obj3;
		}
	} else {
		x1 = obj1;
		y1 = obj2;
		if (defined(obj3.x)) {
			x2 = obj3.x;
			y2 = obj3.y;
		} else {
			x2 = obj3;
			y2 = obj4;
		}
	}
	return distBetweenTwoPoints(x1, y1, x2, y2);
}

_global.defined = function(variable) {
	return typeof(variable) !== "undefined";
}

_global.random = function(max) {
	if (max > 0)
		return Math.floor(Math.random() * max);
}

// some useful array functions
Array.prototype.random = function() {
	if (this.length > 0)
		return this[random(this.length)];
}

Array.prototype.last = function() {
	if (this.length > 0)
		return this[this.length - 1];
}

Array.prototype.filterProperty = function(property, value) {
	return this.filter(function(element) {
		return element[property] === value;
	});
}

// cluster analysis happens here
_global.naiveFindClusters = function(list, size) {
	var ret = { clusters: [], xav: [], yav: [], maxIdx: 0, maxCount: 0 };
	for (var i = list.length - 1; i >= 0; --i) {
		var x = list[i].x, y = list[i].y;
		var found = false;
		for (var j = 0; j < ret.clusters.length; ++j) {
			if (distBetweenTwoPoints(ret.xav[j], ret.yav[j], x, y) < size) {
				var n = ret.clusters[j].length;
				ret.clusters[j][n] = list[i];
				ret.xav[j] = (n * ret.xav[j] + x) / (n + 1);
				ret.yav[j] = (n * ret.yav[j] + y) / (n + 1);
				if (ret.clusters[j].length > ret.maxCount) {
					ret.maxIdx = j;
					ret.maxCount = ret.clusters[j].length;
				}
				found = true;
				break;
			}
		}
		if (!found) {
			var n = ret.clusters.length;
			ret.clusters[n] = [list[i]];
			ret.xav[n] = x;
			ret.yav[n] = y;
			if (1 > ret.maxCount) {
				ret.maxIdx = n;
				ret.maxCount = 1
			}
		}
	}
	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////
})(this);
