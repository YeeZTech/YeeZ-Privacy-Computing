const crypto = require('crypto')
const { randomBytes } = require('crypto')
const aesCmac = require('node-aes-cmac').aesCmac

const secp256k1 = require('secp256k1')
const sha256 = require('js-sha256').sha256
const keccak256 = require('keccak256')

function hashfn(x, y) {
	const version = new Uint8Array(33)

	const sha = sha256.create()

	version[0] = (y[31] & 1) === 0 ? 0x02 : 0x03
	version.set(x, 1)
	sha.update(version)
	return new Uint8Array(sha.array())
}

function gen_ecdh_key_from(skey, pkey) {
	const ecdhPointX = secp256k1.ecdh(pkey, skey, { hashfn }, Buffer.alloc(32))
	return ecdhPointX
}

const YPCCrypto = function () {
	if (!(this instanceof YPCCrypto)) {
		return new YPCCrypto()
	}

	const algorithm = 'aes-128-gcm'
	const aad = Buffer.from('tech.yeez.key.manager')

	//let cmac_key = new Uint8Array(16)
	//cmac_key.set('yeez.tech.stbox')
	let cmac_key = Buffer.from('7965657a2e746563682e7374626f7800', 'hex')

	let derivation_buffer = new Uint8Array(aad.length + 4)
	derivation_buffer[0] = 0x01
	derivation_buffer.set(aad, 1)
	derivation_buffer[aad.length + 1] = 0
	derivation_buffer[aad.length + 2] = 0x80
	derivation_buffer[aad.length + 3] = 0x00
	derivation_buffer = Buffer.from(derivation_buffer)

	this.generatePrivateKey = function () {
		let privKey
		do {
			privKey = randomBytes(32)
		} while (!secp256k1.privateKeyVerify(privKey))
		return privKey
	}

	this.generatePublicKeyFromPrivateKey = function (skey) {
		if (!secp256k1.privateKeyVerify(skey)) {
			alert('invalid private key')
		}
		// false for compress
		// we ignore the first byte, which is '0x04' according to
		// https://davidederosa.com/basic-blockchain-programming/elliptic-curve-keys/;
		const pkey = secp256k1.publicKeyCreate(skey, false).subarray(1)

		return Buffer.from(pkey)
	}

	this.generateAESKeyFrom = function (pkey, skey) {
		if (pkey.length === 64) {
			const prefix = new Uint8Array([0x04])
			pkey = new Uint8Array(pkey)
			pkey = Uint8Array.from([...prefix, ...pkey])
		}
		const shared_key = gen_ecdh_key_from(skey, pkey)
		// The following algorithm is from ypc/stbox/src/tsgx/crypto/ecp.cpp
		const options = { returnAsBuffer: true }
		const key_derive_key = aesCmac(cmac_key, shared_key, options)
		const derived_key = aesCmac(key_derive_key, derivation_buffer, options)
		return derived_key
	}

	this._encryptMessage = function (pkey, skey, msg, prefix) {
		const enc_key = this.generateAESKeyFrom(pkey, skey)
		let iv = randomBytes(12)
		let cipher = crypto.createCipheriv(algorithm, enc_key, iv)
		const tad = new Uint8Array(64)
		tad.set(aad)
		tad[24] = prefix

		cipher.setAAD(Buffer.from(tad), {
			plaintextLength: msg.length
		})
		let encrypted = cipher.update(msg, 'hex', 'hex')
		encrypted += cipher.final('hex')
		encrypted = Buffer.from(encrypted, 'hex')
		const tag = cipher.getAuthTag()
		const length =
			msg.length +
			64 + // public key size
			// sig_size + // sinature_size
			16 + // gcm tag size
		  12 //iv size

		cipher = new Uint8Array(length)
		cipher.set(encrypted)
		cipher.set(iv, encrypted.length)
		cipher.set(this.generatePublicKeyFromPrivateKey(skey), encrypted.length + 12)
		cipher.set(tag, msg.length + 64 + 12)
		return Buffer.from(cipher)
	}

	this.generateForwardSecretKey = function (remote_pkey, skey) {
		const ots = this.generatePrivateKey()

		return this._encryptMessage(remote_pkey, ots, skey, 0x1)
	}
	this.generateEncryptedInput = function (local_pkey, input) {
		const ots = this.generatePrivateKey()
		return this._encryptMessage(local_pkey, ots, input.buffer, 0x2)
	}
	// 调用
	this._decryptMessageWithPrefix = function (skey, msg, prefix) {
		const encrypted = msg.slice(0, msg.length - 64 - 16 - 12)
		const liv = msg.slice(encrypted.length, msg.length - 64 - 16)

		const pkey = msg.slice(encrypted.length + 12, msg.length - 16)
		const tag = msg.slice(msg.length - 16)

		const enc_key = this.generateAESKeyFrom(pkey, skey)
		const decipher = crypto.createDecipheriv(algorithm, enc_key, liv)
		decipher.setAuthTag(tag)
		const tad = new Uint8Array(64)
		tad.set(aad)
		tad[24] = prefix
		decipher.setAAD(Buffer.from(tad), {
			plaintextLength: encrypted.length
		})
		let dec = decipher.update(encrypted, 'hex', 'hex')
		dec += decipher.final('hex')
		return Buffer.from(dec, 'hex')
	}

	this.decryptMessage = function (skey, msg) {
		return this._decryptMessageWithPrefix(skey, msg, 0x2)
	}

	this.decryptForwardMessage = function (skey, msg) {
		return this._decryptMessageWithPrefix(skey, msg, 0x1)
	}

	const eth_hash_prefix = Buffer.from('\x19Ethereum Signed Message:\n32')

	this.generateSignature = function (skey, epkey, ehash) {
		const data = new Uint8Array(epkey.length + ehash.length)

		data.set(epkey, 0)
		data.set(ehash, epkey.length)
		const raw_hash = keccak256(Buffer.from(data))
		let msg = new Uint8Array(eth_hash_prefix.length + raw_hash.length)
		msg.set(eth_hash_prefix)
		msg.set(raw_hash, eth_hash_prefix.length)
		msg = keccak256(Buffer.from(msg))

		const rsig = secp256k1.ecdsaSign(msg, skey)
		const sig = new Uint8Array(65)
		sig.set(rsig.signature)
		sig[64] = rsig.recid + 27
		return Buffer.from(sig)
	}

	this.generateFileNameFromPKey = function (pkey) {
		return pkey.toString('hex').slice(0, 8) + '.json'
	}

	this.generateFileContentFromSKey = function (skey) {
		const c = {}
		c['private_key'] = skey.toString('hex')
		c['public_key'] = this.generatePublicKeyFromPrivateKey(skey).toString('hex')
		return JSON.stringify(c)
	}
}

module.exports = YPCCrypto
