contract_YZDataFactory = {
    'rinkeby':'0xBE588908394c3b4dDEbA2B11B46f24da233b1596',
    'ropsten':'0x93c2168b801E8627CfA725Ba39d9EF0657E88991',
}

contract_ProgramStore = {
    'rinkeby':'0xd886065A4dD60c189aAEFB25CeEFfc04D8FeF3a8',
    'ropsten':'0x0E92004392241f109A6cEd1751C6F903CcD9d22e',
}

map_data_and_request = dict()
map_request_and_data = dict()
request_filters = list()


# keccak256 hash of function `request_proxy()`
hash_request_proxy = '69d18ad7fa2a30c8f532c31aa5f2d08a8e5d8d19c9a87e51cc81e3d6cc4b0238'

# keccak256 hash of function `pkey()`
hash_pkey = '84f720883cdae4b672b88602da3ba45c5b8a3c956e47047313546799ccd91e9a'

# keccak256 hash of function `get_program_info(bytes32)`
hash_get_program_info = '50879237e7a0572dff871b98e3bf0acc71ffcc2e00582206e0511d4d815acd31'


# keccak256 hash of event `NewYZData(address)`
topic_NewYZData = '0x729f473103420c10ca9328745c7f6d9fca3abbe6dfbc6854a4e6c21455d46b4a'

# keccak256 hash of event `RequestData(bytes32,bytes,bytes,bytes,bytes32,uint256,bytes)`
topic_RequestData = '0xa2eb61ce32ba0f2b61d32b2505b63c7d91843928753a586e5a3d0feaf08948ff'


PARAMS_BYTES = 32
PARAMS_BYTES_IN_HEX_LENGTH = 64

DAEMON_TIMER_IN_SECONDS = 30
