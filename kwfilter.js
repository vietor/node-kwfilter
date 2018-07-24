var fs = require('fs');

function tryRequire(module) {
    try {
        return require(module);
    } catch (e) {
        return null;
    }
}

function tryLoadModule(relative, name) {
    var node = tryRequire(relative + name);
    if (!node)
        node = tryRequire(relative + '/' + name + '-' + process.arch);
}

var kwfilter = tryRequire('./build/Release/kwfilter');
if (!kwfilter)
    kwfilter = tryLoadModule('./build/' + process.platform, 'kwfilter');
if (!kwfilter)
    kwfilter = tryLoadModule('./build/' + process.platform + '/' + process.version, 'kwfilter');
if (!kwfilter)
    kwfilter = tryLoadModule('./build/' + process.platform + '/' + (process.version.split('.').slice(0, 2).join('.')), 'kwfilter');

if (!kwfilter)
    throw new Error("not found module for " + process.platform + '(' + process.arch + ')');

exports.newInstance = function(keywords, mode) {
    return kwfilter.KeywordFilter(keywords, mode || 0);
};
