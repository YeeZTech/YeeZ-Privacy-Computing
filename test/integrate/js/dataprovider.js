const nReadlines = require('n-readlines');
const fs = require('fs');
const ByteBuffer = require('bytebuffer');
const keccak256 = require('keccak256');
const YPCCrypto = require('./ypccrypto.js')();
const YPCNtObject = require('./ypcntobject.js')();

const max_item_size = 64 * 1024;
const BlockFile = require('./blockfile.js')(Buffer.from('1fe2ef7f3ed18847', 'hex'), 1024 * 1024, 256 * max_item_size);

const DataProvider = function() {
	if (!(this instanceof DataProvider)) {
		return new DataProvider()
	}

  function batch2ntpackage(batch) {
    buf_size = 4 + 8 * batch.length;
    for (let i = 0; i < batch.length; i++) {
      buf_size += 8;
      buf_size += batch[i].length;
    }

    let buf = new ByteBuffer(buf_size, ByteBuffer.LITTLE_ENDIAN)
    offset = 0;
    // package id
    buf.writeUint32(0x82c4e8d8, offset);
    offset += 4;
    // batch size
    buf.writeUint64(batch.length, offset);
    offset += 8;
    // batch items
    for (let i = 0; i < batch.length; i++) {
      buf.writeUint64(batch[i].length, offset);
      offset += 8;
      buf.append(batch[i], offset);
      offset += batch[i].length;
    }
    return buf.buffer;
  }

  function write_batch(fd, batch, public_key) {
    let pkg_bytes = batch2ntpackage(batch);
  	const ots = YPCCrypto.generatePrivateKey()
    s = YPCCrypto._encryptMessage(Buffer.from(public_key, 'hex'), ots, pkg_bytes, 0x2);
    BlockFile.append_item(fd, s);
  }

  function ignore_leading_and_tailing_spaces(item) {
    let idx = 0;
    while (item[idx] ===  ' ') {
      idx++;
    }
    item = item.substring(idx);
    idx = item.length - 1;
    while (item[idx] === ' ') {
      idx--;
    }
    item = item.substring(0, idx + 1);
    return item;
  }

  this.sealFile = function(dataUrl, csvConfig, usePublickeyFile, sealedDataUrl, output) {
    const data_lines = new nReadlines(dataUrl);
    const csv_schema = JSON.parse(fs.readFileSync(csvConfig));
    const key_file = JSON.parse(fs.readFileSync(usePublickeyFile));

    let data_hash = keccak256(Buffer.from('Fidelius', 'utf-8'))
    let line, counter = 0;
    let batch = [], batch_size = 0;
    let fd = BlockFile.open_for_write(sealedDataUrl);

    while (line = data_lines.next()) {
      line = line.toString().replace('\r', '').replace('\t', '');
      items = line.split(',');
      schema = csv_schema.schema;
      input = [];
      for (let i = 0; i < schema.length; i++) {
        item = ignore_leading_and_tailing_spaces(items[i]);
        obj = {'type':schema[i].type, 'value':item};
        if (obj.type === 'double') {
          obj.value = parseFloat(obj.value);
        }
        input.push(obj);
      }
      ntInput = YPCNtObject.generateBytes(input);
      batch.push(ntInput.buffer);
      batch_size += ntInput.buffer.length;
      if (batch_size >= max_item_size) {
        write_batch(fd, batch, key_file['public-key']);
        batch = [];
        batch_size = 0;
      }
      k = Buffer.from(data_hash.toString('hex') + ntInput.buffer.toString('hex'), 'hex');
      data_hash = keccak256(k);
      counter++;
    }
    console.log('data hash:', data_hash.toString('hex'))

    if (batch.length != 0) {
      write_batch(fd, batch, key_file['public-key']);
    }
    BlockFile.close_file(fd);

    let output_content = '';
    output_content += ('data_url = ' + dataUrl + '\n');
    output_content += ('sealed_data_url = ' + sealedDataUrl + '\n');
    output_content += ('public_key = ' + key_file['public-key'] + '\n');
    output_content += ('data_id = ' + data_hash.toString('hex') + '\n');
    output_content += ('item_num = ' + counter + '\n');
    fs.writeFileSync(output, output_content);
  }
}

module.exports = DataProvider
