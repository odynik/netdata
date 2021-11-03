# -*- coding: utf-8 -*-
# Description: example netdata python.d module
# Author: Put your name here (your github login)
# SPDX-License-Identifier: GPL-3.0-or-later
from bases.FrameworkServices.SimpleService import SimpleService

priority = 90000

ORDER = [
    'inorder'
    ]

CHARTS = {
    'inorder': {
        'options': [None, 'A sequence of ascending numbers', 'Asc number', 'inorder', 'inorder', 'line'],
        'lines': [
            ['ascInOrder0']
        ]
    }    
}

class Service(SimpleService):
    def __init__(self, configuration=None, name=None):
        SimpleService.__init__(self, configuration=configuration, name=name)
        self.order = ORDER
        self.definitions = CHARTS
        self.num_lines = self.configuration.get('num_lines', 1)
        self.lower = self.configuration.get('lower', 0)
        self.upper = self.configuration.get('upper', 4096)
        self.update_every = self.configuration.get('update_every', 5)
        self.current = None

    @staticmethod
    def check():
        return True

    def get_data(self):
        data = dict()
        for i in range(0, self.num_lines):
            dimension_id = ''.join(['ascInOrder', str(i)])
            if dimension_id not in self.charts['inorder']:
                self.charts['inorder'].add_dimension([dimension_id])
            data[dimension_id] = self.incr(self.lower, self.upper)
        return data
           
    def incr(self, lower, upper):
        if(self.current is None) or (self.current == upper):
            self.current = lower
        else:
            self.current += self.update_every
        return self.current