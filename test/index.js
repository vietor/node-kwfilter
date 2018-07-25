var kwfilter = require('../');

var words2 = ["a"],
    text2 = "aaaaaaaaa";
var kf2 = kwfilter.newInstance(words2);
console.log('\%j on %j', words2, text2);
console.log('keyword filter: %j', kf2.filter(text2, '*'));
console.log('keyword render: %j', kf2.render(text2, "<", ">"));

var words3 = ["fuck", "sex", "fu"],
    text3 = "bad words like: fUCk, sex, pron, cafu...";
var kf3 = kwfilter.newInstance(words3);
console.log('\n%j on %j', words3, text3);
console.log('keyword exitst: %j', kf3.exists(text3));
console.log('keyword filter: %j', kf3.filter(text3, '*'));
console.log('keyword parser: %j', kf3.parser(text3));
