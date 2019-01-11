var kwfilter = require('../');

console.log("======== test1 ========");
var words1 = ["a"],
    texts1 = ["aaaa", "abab", "baba"];
var kf1 = kwfilter.newInstance(words1);
texts1.forEach(function(text1) {
    console.log('%j on %j', words1, text1);
    console.log('keyword filter: %j', kf1.filter(text1, '*'));
    console.log('keyword render: %j', kf1.render(text1, "<", ">"));
});

console.log("======== test2 ========");
var words2 = ["ab"],
    texts2 = ["aaaa", "abcabc", "acabcacbcabc"];
var kf2 = kwfilter.newInstance(words2);
texts2.forEach(function(text2) {
    console.log('%j on %j', words2, text2);
    console.log('keyword filter: %j', kf2.filter(text2, '*'));
    console.log('keyword render: %j', kf2.render(text2, "<", ">"));
});

console.log("======== test3 ========");
var words3 = ["fuck", "sex", "fu"],
    text3 = "bad words like: fUCk, sex, pron, cafu...";
var kf3 = kwfilter.newInstance(words3);
console.log('%j on %j', words3, text3);
console.log('keyword exists: %j', kf3.exists(text3));
console.log('keyword filter: %j', kf3.filter(text3, '*'));
console.log('keyword filter3: %j', kf3.filter(text3, '*', 3));
console.log('keyword render: %j', kf3.render(text3, '<', '>'));
console.log('keyword parser: %j', kf3.parser(text3));

console.log("======== test4 ========");
var words4 = ["ab", "ac"],
    texts4 = ["aaaa", "aaacaa", "aaaabc"];
var kf4 = kwfilter.newInstance(words4);
texts4.forEach(function(text4) {
    console.log('%j on %j', words4, text4);
    console.log('keyword exists: %j', kf4.exists(text4));
});

console.log("======== test5 ========");
var words5 = ["ass", "uc"],
    texts5 = ["assemblicy", "fuck", "ass", "uc", "haha ass cc uc xuc uc"];
var kf5 = kwfilter.newInstance(words5, 1);
texts5.forEach(function(text5) {
    console.log('%j on %j', words5, text5);
    console.log('keyword exists: %j', kf5.exists(text5));
    console.log('keyword filter: %j', kf5.filter(text5, '*'));
});
