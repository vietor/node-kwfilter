var kwfilter = require('../');

var words1 = ["a"],
    texts1 = ["aaaa", "abab", "baba"];
var kf1 = kwfilter.newInstance(words1);

texts1.forEach(function(text1) {
    console.log('%j on %j', words1, text1);
    console.log('keyword filter: %j', kf1.filter(text1, '*'));
    console.log('keyword render: %j', kf1.render(text1, "<", ">"));
});

var words2 = ["ab"],
    texts2 = ["aaaa", "abcabc", "acabcacbcabc"];
var kf2 = kwfilter.newInstance(words2);
texts2.forEach(function(text2) {
    console.log('%j on %j', words2, text2);
    console.log('keyword filter: %j', kf2.filter(text2, '*'));
    console.log('keyword render: %j', kf2.render(text2, "<", ">"));
});

var words3 = ["fuck", "sex", "fu"],
    text3 = "bad words like: fUCk, sex, pron, cafu...";
var kf3 = kwfilter.newInstance(words3);
console.log('%j on %j', words3, text3);
console.log('keyword exists: %j', kf3.exists(text3));
console.log('keyword filter: %j', kf3.filter(text3, '*'));
console.log('keyword render: %j', kf3.render(text3, '<', '>'));
console.log('keyword parser: %j', kf3.parser(text3));
