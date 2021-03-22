import logging


logger = logging.getLogger('ypc-data-request')
log_handler = logging.FileHandler('./ypc-data-request.log')
formatter = logging.Formatter('%(asctime)s %(filename)s:%(lineno)s %(funcName)s() [%(levelname)s]  %(message)s')
log_handler.setFormatter(formatter)
logger.addHandler(log_handler)
logger.setLevel(logging.INFO)
