const util = require('util');
const execCore = util.promisify(require('child_process').exec);

function sleep(ms) {
  return new Promise((resolve, reject) => {
    setTimeout(resolve, ms);
  });
}

async function exec(command) {
  try {
    return await execCore(command);
  } catch (err) {
    const stderr = err.message;
    return {stderr};
  }
}


async function notarizeApp(password) {
  //
  const command = `xcrun altool --notarize-app -t osx -f WizNote.zip --primary-bundle-id="cn.wiz.wiznoteformac" -u "weishijun@msn.com" -p ${password}`;
  console.log(command);

  //
  const getUuidFromMessage = (stderr) => {
    if (stderr) {

    }
  }
  //
  const { stdout, stderr} = await exec(command);
  //
  if (stderr) {
    const info = 'The software asset has already been uploaded. The upload ID is ';
    const index = stderr.indexOf(info);
    if (index != -1) {
      let uuid = stderr.substr(index + info.length);
      const parts = uuid.split(' ');
      uuid = parts[0];
      if (uuid.endsWith('"')) {
        uuid = uuid.substr(0, uuid.length - 1);
      }
      return uuid;
    }
    console.error(stderr);
    return;
  }

  console.log('altool output:', stdout);
  //
  const index = stdout.indexOf('=');
  if (index > -1) {    
    return stdout.substr(index + 1).trim();
  }
  //
  return '';
}
//
async function doNotarize(password) {
  const uuid = await notarizeApp(password);
  if (!uuid) {
    return;
  }
  //
  console.log('uuid: ' + uuid);
  //
  while (true) {
    //
    console.log('wait 10 seconds');
    await sleep(1000 * 10);
    //
    const command = `xcrun altool --notarization-info ${uuid} -u "weishijun@msn.com" -p "${password}"`;
    console.log(command);
    const {stdout, stderr} = await exec(command);
    //
    if (stderr) {
      console.error(stderr);
      return;
    }
    //
    console.log(stdout);
    //Status Message: Package Approved
    if (stdout.indexOf('Status Message: Package Approved') != -1) {
      break;
    }
    //
  }
  //
  const command = `xcrun stapler staple "WizNote.app"`;
  console.log(command);
  const {stdout, stderr} = await exec(command);
  if (stderr) {
    console.error(stderr);
  } else {
    console.log(stdout);
  }
}

const password = process.argv[2];
doNotarize(password);