var fs = require('fs');

function tryRequire(module) {
    try {
        return require(module);
    } catch (e) {
        return null;
    }
}

function tryLoadModule(relative, name, suffix) {
    var node = tryRequire(relative + name + '-' + suffix + '-' + process.arch);
    if (!node)
        node = tryRequire(relative + name + '-' + suffix);
    if (!node)
        node = tryRequire(relative + suffix + '-' + process.arch);
    if (!node)
        node = tryRequire(relative + suffix);
}

var kwfilter = tryRequire('./build/Release/kwfilter');
if (!kwfilter)
    kwfilter = tryLoadModule('./build/', 'kwfilter', process.platform + '-' + process.version);
if (!kwfilter)
    kwfilter = tryLoadModule('./build/', 'kwfilter', process.platform + '-' + (process.version.split('.').slice(0, 2).join('.')));
if (!kwfilter)
    kwfilter = tryLoadModule('./build/', 'kwfilter', process.platform);

if (!kwfilter)
    throw new Error("not found module for " + process.platform + '-' + process.version + '-' + process.arch);

exports.newInstance = function(keywords, mode) {
    return kwfilter.KeywordFilter(keywords, mode || 0);
};
