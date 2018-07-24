var kwfilter = require('../');

var origin = "KWFILTER1234567890kwfilter";

var kf = kwfilter.newInstance([origin.charAt(0), origin.charAt(origin.length - 1)]);
console.log('keyword exists: %j, filter: %j', kf.exists(origin), kf.filter(origin, '*'));

var kf2 = kwfilter.newInstance(["a"]);
console.log('keyword "a" sequence filter: %j', kf2.filter("aaaaaaaaa", '*'));
console.log('keyword "a" sequence render: %j', kf2.render("aaaaaaaaa", "<", ">"));

var kf3 = kwfilter.newInstance(["fuck", "sex"]);
console.log('keyword parser: %j', kf3.parser("bad words like: fuck, sex, pron ..."));
