const { program } = require('commander');
const fs = require('fs');
const nReadlines = require('n-readlines');
const DataProvider = require('./dataprovider.js')();

program
  .description('YeeZ Privacy Data Hub')
  .option('--name <string>', 'name')
  .option('--data-url <string>', 'data file path')
  .option('--config <string>', 'JSON configuration file')
  .option('--use-publickey-file <string>', 'public key file path')
  .option('--dian-public-key <string>', 'DIAN public key')
  .option('--sealed-data-url <string>', 'sealed data file path')
  .option('--output <string>', 'data meta output');

program.parse();
const options = program.opts();

if (!options.dataUrl) {
  console.log('option --data-url not specified!');
  return;
}
if (!options.usePublickeyFile) {
  console.log('option --use-publickey-file not specified!');
  return;
}
if (!options.sealedDataUrl) {
  console.log('option --sealed-data-url not specified!');
  return;
}
if (!options.output) {
  console.log('option --output not specified!');
  return;
}

const data_lines = new nReadlines(options.dataUrl);
// TODO dian public key should specify
DataProvider.init(options.name, options.dianPublicKey, data_lines);
const key_file = JSON.parse(fs.readFileSync(options.usePublickeyFile))
let all = DataProvider.sealFile(key_file);

fd = fs.openSync(options.sealedDataUrl, 'w');
let buf = all[0].buffer;
fs.writeSync(fd, buf, 0, buf.length, 0);
fs.closeSync(fd);

fs.writeFileSync(options.output, all[1]);
