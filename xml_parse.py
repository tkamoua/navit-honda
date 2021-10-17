#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Oct 16 16:49:47 2021

@author: samephraim
"""
import xml.etree.ElementTree as ET
import time

tree = ET.parse('navit.xml')
root = tree.getroot()
counter = 0
while True:
    time.sleep(30)
    for button in root.iter('osd'):
        if "name" in button.attrib:
            if button.attrib['name'] == "current_wx":
                counter = counter + 1
                if counter%2 == 1:
                    button.attrib['src'] = "$HOME/navit-build/navit/icons/snow.svg"
                else:
                    button.attrib['src'] = "$HOME/navit-build/navit/icons/clear_day.svg"
            
            
tree.write("navit.xml")