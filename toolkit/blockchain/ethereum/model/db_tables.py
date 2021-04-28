from flask import Flask
from flask_sqlalchemy import SQLAlchemy
from sqlalchemy import Table, Column, Integer, String, Text, Boolean, Float
import db_conf


app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = db_conf.DB_URI
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = True
# db = SQLAlchemy(app, session_options={'autocommit':True})
db = SQLAlchemy(app)


class Contract(db.Model):
    __tablename__ = 'Contract'
    id = db.Column('id', Integer, primary_key=True)
    yz_data = db.Column('YZData', String(40))
    yz_request = db.Column('YZDataRequest', String(40))


class RequestData(db.Model):
    __tablename__ = 'RequestData'
    id = db.Column('id', Integer, primary_key=True)
    request_hash = db.Column('request_hash', String(64))
    encrypted_skey = db.Column('encrypted_skey', Text)
    encrypted_input = db.Column('encrypted_input', Text)
    provider_pkey = db.Column('provider_pkey', String(128))
    analyzer_pkey = db.Column('analyzer_pkey', String(128))
    program_enclave_hash = db.Column('program_enclave_hash', String(64))
    forward_sig = db.Column('forward_sig', String(130))
    status = db.Column('status', Integer)
    encrypted_result = db.Column('encrypted_result', Text)
    result_signature = db.Column('result_signature', String(130))
    data_hash = db.Column('data_hash', String(64))


if __name__ == '__main__':
    db.create_all()
