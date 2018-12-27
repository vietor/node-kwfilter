# node-kwfilter

A keyword filter C++ Addons for NodeJS

## Install
`npm install kwfilter --save`

## Usage

```javascript
var kwfilter = require('kwfilter');

var words3 = ["fuck", "sex", "fu"],
    text3 = "bad words like: fUCk, sex, pron, cafu...";
var kf3 = kwfilter.newInstance(words3);
console.log('%j on %j', words3, text3);
console.log('keyword exists: %j', kf3.exists(text3));
console.log('keyword filter: %j', kf3.filter(text3, '*'));
console.log('keyword render: %j', kf3.render(text3, '<', '>'));
console.log('keyword parser: %j', kf3.parser(text3));
```

### Output

``` bash
["fuck","sex","fu"] on "bad words like: fUCk, sex, pron, cafu..."
keyword exists: true
keyword filter: "bad words like: ****, ***, pron, ca**..."
keyword render: "bad words like: <fUCk>, <sex>, pron, ca<fu>..."
keyword parser: [{"pos":16,"count":4},{"pos":22,"count":3},{"pos":35,"count":2}]
```

## API

### newInstance(keywords, [mode])

Create a **kwfilter** instance.

> mode = 1, the **word** mode.

### exists(text)

Check if **keyword** in the **text**.

### filter(text, cover, [border])

Replace the **keyword** in **text** with **cover character**.

### render(text, prefix, stuffix)

Wrap the **keyword** in **text** with **prefix** and **stuffix**.

### parser(text)

Parse the **keyword** in **text** 's positions.
