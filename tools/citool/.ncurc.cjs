const fs = require("fs");
const path = require("path");

function getMinReleaseAge() {
  try {
    const npmrc = fs.readFileSync(path.join(__dirname, ".npmrc"), "utf8");
    const match = npmrc.match(/^min-release-age=(\d+)/m);
    return match ? parseInt(match[1], 10) : 0;
  } catch {
    return 0;
  }
}

module.exports = {
  cooldown: getMinReleaseAge(),
};
