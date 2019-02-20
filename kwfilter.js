var fs = require('fs');

function tryRequire(module) {
    console.log(module);
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
    return node;
}

var binding = tryRequire('./build/Release/kwfilter');
if (!binding) {
    var versions = [process.version, process.version.split('.').slice(0, 2).join('.'), process.version.split('.')[0]];
    for (var i = 0, len = versions.length; i < len && !binding; ++i) {
        binding = tryLoadModule('./build/', 'kwfilter', process.platform + '-' + versions[i]);
    }
}

if (!binding)
    throw new Error("not found module for " + process.platform + '-' + process.version + '-' + process.arch);

exports.newInstance = function(keywords, mode) {
    return binding.KeywordFilter(keywords, mode || 0);
};
