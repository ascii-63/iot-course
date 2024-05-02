""" from .extension import db

class data(db.Model):
    id = db.Column(db.Integer, primary_key = True)
    name = db.Column(db.String(100))
    P = db.Column(db.Float)
    U = db.Column(db.Float)
    I = db.Column(db.Float)
    status = db.Column(db.Boolean)
    
    def __init__(self, name, P, U, I, status):
        self.name = name
        self.P = P
        self.U = U
        self.I = I
        self.status = status
        

 """