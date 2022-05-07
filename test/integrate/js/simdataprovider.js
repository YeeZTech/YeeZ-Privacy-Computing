const { program } = require('commander');
const DataProvider = require('./dataprovider.js')();

program
  .description('YeeZ Privacy Data Hub')
  .option('--data-url <string>', 'data file path')
  .option('--config <string>', 'JSON configuration file')
  .option('--use-publickey-file <string>', 'public key file path')
  .option('--sealed-data-url <string>', 'sealed data file path')
  .option('--output <string>', 'data meta output');

program.parse();
const options = program.opts();

if (!options.dataUrl) {
  console.log('option --data-url not specified!');
  return;
}
if (!options.config) {
  console.log('option --config not specified!');
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

DataProvider.sealFile(options.dataUrl, options.config, options.usePublickeyFile, options.sealedDataUrl, options.output);
