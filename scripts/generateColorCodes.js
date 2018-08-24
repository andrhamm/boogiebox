// Generates 256 4-button combinations

// these values represent the ID of the button
// this assumes button colors are static
const colors = ["0", "1", "2", "3"];

function cartesian() {
    // https://stackoverflow.com/a/15310051/169717
    var r = [], arg = arguments, max = arg.length-1;
    function helper(arr, i) {
        for (var j=0, l=arg[i].length; j<l; j++) {
            var a = arr.slice(0); // clone arr
            a.push(arg[i][j]);
            if (i==max)
                r.push(a);
            else
                helper(a, i+1);
        }
    }
    helper([], 0);
    return r;
}

const results = cartesian(colors, colors, colors, colors);

// sort so less repetitive codes come first,
// prefer left-to-right button ordering
const sorted = results.sort((a,b) => {
  const mapVals = (map, val) => {
    if (map[val] === undefined) {
      map[val] = 1;
    } else {
      map[val] += 1;
    }
    return map;
  };
  const aMap = a.reduce(mapVals, {});
  const bMap = b.reduce(mapVals, {});

  const aDupeCount = Object.values(aMap).sort().reverse()[0];
  const bDupeCount = Object.values(bMap).sort().reverse()[0];

  const acomp = +(a.join(''));
  const bcomp = +(b.join(''));

  return aDupeCount === bDupeCount ? (acomp > bcomp ? 1 : -1) : (aDupeCount > bDupeCount) ? 1 : -1;
});

// console.log(sorted);
console.log(sorted.length);
console.log(JSON.stringify(sorted.map((code,i) => `${i+1},${code.join('')}`).join("\n")));
