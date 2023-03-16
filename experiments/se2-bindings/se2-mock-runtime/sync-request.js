// Make synchronous network requests using another node process,
// https://nodejs.org/dist/latest-v18.x/docs/api/child_process.html#synchronous-process-creation and
// https://nodejs.org/dist/latest-v18.x/docs/api/https.html#httpsgeturl-options-callback

const nodeBin = process.argv[0];

const { execFileSync } = require("child_process");

function get(url) {
  return execFileSync(
    nodeBin,
    ["-e", `require("https").get("${url}", res => res.pipe(process.stdout) )`],
    {
      maxBuffer: 8 * 1024 * 1024, // 8MB
    }
  );
}

module.exports = { get };
