# -*- coding: utf-8 -*-
# Description: example netdata python.d module
# Author: Put your name here (your github login)
# SPDX-License-Identifier: GPL-3.0-or-later

from os import name
from bases.FrameworkServices.SimpleService import SimpleService

priority = 90000

ORDER = [
    'increment_xX',
]

CHARTS = {
    'increment_xX': {
        'options': [None, 'An incremental value xX', 'Asc#', 'increment', 'test_increment.xX', 'line'],
        'lines': [
            ['xX'],
        ]
    },
}

class Service(SimpleService):
    def __init__(self, configuration=None, name=None):
        SimpleService.__init__(self, configuration=configuration, name=name)
        self.order = ORDER
        self.definitions = CHARTS
        self.num_lines = self.configuration.get('num_lines', 1)
        self.chname_template = '_'.join(self.name.split('_')[1:])
        self.print_debug = False
        self.import_settings()
        # TODO: Remove the print debug shit and import the python.d.plugin logger ;) If you leave print there...
        if self.print_debug:
            print("Config Name: " + str(self.name) +", JobName: "+str(self.job_name))
            print("Chart Name template: " + str(self.chname_template))        

    @staticmethod
    def check():
        return True
    
    def import_settings(self):
        for i in range(0, len(self.order)):
            if self.order[i] == 'increment_xX':
                self.order[i] = self.chname_template
            elif self.chname_template not in self.order:
                self.order.append(self.chname_template)
        if self.chname_template not in self.definitions.keys():
            self.definitions[self.chname_template] = self.chart_param(self.chname_template)
        self.lower = self.configuration.get("lower")
        self.upper = self.configuration.get("upper")
        self.counter = self.lower - self.update_every
        if self.print_debug:
            print("Order: " + str(self.order))            
            print("Definitions: " + str(self.definitions))                        
            print("Name: "+self.chname_template+"|Dimension: "+self.dimension_id)
            print("Charts: "+str(self.charts))
            print("Import settings: name="+ str(self.chname_template) +", ue=" + str(self.update_every)+", dim=" + str(self.dimension_id))

    def get_data(self):
        data = dict()
        data[self.dimension_id] = self.increment_step()
        if self.print_debug:
            print(self.chname_template + ": Data[" + self.dimension_id + "]=" + str(data))
        return data
    
    def increment_step(self):
        if self.counter >= self.upper:
            self.counter = self.lower
        else:
            self.counter += self.update_every
        return self.counter
    
    def chart_param(self, chart):
        chart_params = {
        'options': [None, 'An incremental value xX', 'Asc#', 'increment', 'test_increment.xX', 'line'],
        'lines': [ ['xX'], ]
        }
        if 'increment_xX' in self.definitions:
            self.definitions.pop('increment_xX')
        self.dimension_id = chart.split('_')[1]
        chart_params['lines'] = [[self.dimension_id, None, 'absolute', 1, 1],]
        chart_params['options'][1] = 'An incremental value ' + self.dimension_id
        chart_params['options'][4] = 'test_increment.' + self.dimension_id
        if self.print_debug:
            print("CHART PARAM for {0}: {1}".format(chart, chart_params))
        return chart_params
