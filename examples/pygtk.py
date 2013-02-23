#!/usr/bin/env python
# -*- coding: utf-8 -*-                                                         
                                                                                
# Copyright (C) 2013 Deepin, Inc.                                               
#               2013 Zhai Xiang                                                 
#                                                                               
# Author:     Zhai Xiang <zhaixiang@linuxdeepin.com>                            
# Maintainer: Zhai Xiang <zhaixiang@linuxdeepin.com>                            
#                                                                               
# This program is free software: you can redistribute it and/or modify          
# it under the terms of the GNU General Public License as published by          
# the Free Software Foundation, either version 3 of the License, or             
# any later version.                                                            
#                                                                               
# This program is distributed in the hope that it will be useful,               
# but WITHOUT ANY WARRANTY; without even the implied warranty of                
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 
# GNU General Public License for more details.                                  
#                                                                               
# You should have received a copy of the GNU General Public License             
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import gtk
import pypulse
from deepin_utils.process import run_command

def value_changed(widget):
    print "DEBUG value_changed", widget.value
    # TODO: 直接调用命令行
    # 改变输出音量, 重点需要测试set_xxx, 若修正了FIXME的BUG, 请注释掉该方法
    run_command("pacmd set-sink-volume %d %d" % (1, widget.value * 1000))
    
    # FIXME: 多次释放指针对象错误 :(
    # *** glibc detected *** python: double free or corruption (fasttop): 0x0000000002fb5860 ***
    '''
    current_sink = pypulse.get_fallback_sink_index()                        
    if current_sink is None:                                                
        print "DEBUG current_sink is None"
        return                                                              
    volume_list = pypulse.PULSE.get_output_volume_by_index(current_sink)       
    channel_list = pypulse.PULSE.get_output_channels_by_index(current_sink) 
    if not volume_list or not channel_list:                                 
        print "DEBUG volume_list or channel_list is None"
        return                                                              
    balance = pypulse.get_volume_balance(channel_list['channels'], volume_list, channel_list['map'])
    volume = int(widget.value / 100.0 * pypulse.NORMAL_VOLUME_VALUE)
    print "DEBUG speaker volumel set:", balance, volume                           
    pypulse.PULSE.set_output_volume_with_balance(current_sink, volume, balance)
    '''

def destroy(*args):
    """ Callback function that is activated when the program is destoyed """
    gtk.main_quit()

window = gtk.Window(gtk.WINDOW_TOPLEVEL)
window.set_size_request(300, 200)
window.connect("destroy", destroy)
window.set_border_width(10)

# TODO: 测试通过 :)
print pypulse.PULSE.get_cards()

adjust = gtk.Adjustment(value=90, lower=0, upper=120, step_incr=1, page_incr=1)
adjust.connect("value-changed", value_changed)
hscale = gtk.HScale(adjust)
window.add(hscale)
hscale.show()

window.show_all()
gtk.main()
