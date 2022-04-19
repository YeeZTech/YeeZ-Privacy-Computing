const YPCCrypto = require('./ypccrypto.js')()
const YPCNtObject = require('./ypcntobject.js')()
//var skey = YPCCrypto.generatePrivateKey()
//var pkey = YPCCrypto.generatePublicKeyFromPrivateKey(skey)
//var encrypted_secret = YPCCrypto.generateForwardSecretKey(pkey, skey)
//var ehash = skey
//var sig = YPCCrypto.generateSignature(skey, skey, pkey, ehash)
//var encrypted_input = YPCCrypto.generateEncryptedInput(local_pkey, input)

var argv = require("yargs").argv
const fs = require('fs')



function main(){
  if(argv.genKey){
    console.log('simjs library genKey')
    skey = YPCCrypto.generatePrivateKey()
    pkey = YPCCrypto.generatePublicKeyFromPrivateKey(skey)
    let obj={"private-key":skey.toString('hex'),
    'public-key':pkey.toString('hex')}
    let json =JSON.stringify(obj)
    fs.writeFileSync(argv.output, json)
  }
  if(argv.dhash){
    console.log('simjs library dhash')
    console.log('argv, ', argv)
    let obj = {}
    tee_pkey = Buffer.from(argv.teePubkey, 'hex')
    ehash = Buffer.from(argv.useEnclaveHash, 'hex')
    console.log('argv use param', argv.useParam)
    input_buf = YPCNtObject.generateBytes(JSON.parse(argv.useParam))
    console.log('input buf:', input_buf)
    //input = Buffer.from(argv.useParam.toString())
    key_pair = JSON.parse(fs.readFileSync(argv.usePrivatekeyFile))
    shu_skey = Buffer.from(key_pair['private-key'], 'hex')
    console.log("shu_skey: ", shu_skey.toString('hex'))
    console.log(shu_skey.length)
    shu_pkey = Buffer.from(key_pair['public-key'], 'hex')
    console.log("shu_pkey: ", shu_pkey)
    console.log(shu_pkey.length)
    encrypted_secret = YPCCrypto.generateForwardSecretKey(tee_pkey, shu_skey)
    sig = YPCCrypto.generateSignature(shu_skey, shu_skey, tee_pkey, ehash)
    encrypted_input = YPCCrypto.generateEncryptedInput(shu_pkey, input_buf)

    obj["encrypted-input"] = encrypted_input.toString('hex')
    obj['forward-sig'] = sig.toString('hex')
    obj['encrypted-skey'] = encrypted_secret.toString('hex')
    obj['analyzer-pkey'] = shu_pkey.toString('hex')
    obj['program-enclave-hash'] = ehash.toString('hex')
    obj['provider-pkey'] = tee_pkey.toString('hex')

    let json = JSON.stringify(obj)
    fs.writeFileSync(argv.output, json)
  }
  if(argv.forward){
    console.log('simjs library forward')
    console.log('argv, ', argv)
    use_privatekey_file = JSON.parse(fs.readFileSync(argv.usePrivatekeyFile))
    tee_pkey = Buffer.from(argv.teePubkey, 'hex')
    use_enclave_hash = Buffer.from(argv.useEnclaveHash, 'hex')

    shu_skey = Buffer.from(use_privatekey_file['private-key'], 'hex')
    encrypted_secret = YPCCrypto.generateForwardSecretKey(tee_pkey, shu_skey)
    sig = YPCCrypto.generateSignature(shu_skey, tee_pkey, use_enclave_hash)

    let obj = {}
    obj['enclave_hash'] = use_enclave_hash.toString('hex')
    obj['forward_sig'] = sig.toString('hex')
    obj['encrypted_skey'] = encrypted_secret.toString('hex')

    let json = JSON.stringify(obj)
    fs.writeFileSync(argv.output, json)
  }
  if(argv.request){
    console.log('simjs library request')
    console.log('argv, ', argv)
    input_buf = YPCNtObject.generateBytes(JSON.parse(argv.useParam))
    use_publickey_file = JSON.parse(fs.readFileSync(argv.usePublickeyFile))

    shu_pkey = Buffer.from(use_publickey_file['public-key'], 'hex')
    encrypted_input = YPCCrypto.generateEncryptedInput(shu_pkey, input_buf)

    let obj = {}
    obj['encrypted-input'] = encrypted_input.toString('hex')
    obj['analyzer-pkey'] = shu_pkey.toString('hex')

    let json = JSON.stringify(obj)
    fs.writeFileSync(argv.output, json)
  }

}

main()
