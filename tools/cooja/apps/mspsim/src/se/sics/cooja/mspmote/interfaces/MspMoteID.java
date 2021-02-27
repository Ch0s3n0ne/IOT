/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

package se.sics.cooja.mspmote.interfaces;
import java.util.Observable;
import java.util.Observer;

import javax.swing.JLabel;
import javax.swing.JPanel;

import org.apache.log4j.Logger;

import se.sics.cooja.Mote;
import se.sics.cooja.interfaces.MoteID;
import se.sics.cooja.mspmote.MspMote;
import se.sics.cooja.mspmote.MspMoteMemory;
import se.sics.mspsim.core.Memory;
import se.sics.mspsim.core.MemoryMonitor;

/**
 * Mote ID.
 *
 * @author Fredrik Osterlind
 */
public class MspMoteID extends MoteID {
	private static Logger logger = Logger.getLogger(MspMoteID.class);

	private MspMote mote;
	private MspMoteMemory moteMem = null;

	private boolean writeFlashHeader = true;
	private int moteID = -1;

	private MemoryMonitor memoryMonitor;
	
	/**
	 * Creates an interface to the mote ID at mote.
	 *
	 * @param mote ID
	 * @see Mote
	 * @see se.sics.cooja.MoteInterfaceHandler
	 */
	public MspMoteID(Mote m) {
		this.mote = (MspMote) m;
		this.moteMem = (MspMoteMemory) mote.getMemory();
	}

	public int getMoteID() {
		return moteID;
	}

	public void setMoteID(int newID) {
		if (moteID != newID) {
			mote.idUpdated(newID);
			setChanged();
		}
		moteID = newID;

		if (moteMem.variableExists("node_id")) {
			moteMem.setIntValueOf("node_id", moteID);

			if (writeFlashHeader) {
				/* Write to external flash */
				SkyFlash flash = mote.getInterfaces().getInterfaceOfType(SkyFlash.class);
				if (flash != null) {
					flash.writeIDheader(moteID);
				}
				writeFlashHeader = false;
			}
			/* Experimental: set Contiki random seed variable if it exists */
			if (moteMem.variableExists("rseed")) {
				moteMem.setIntValueOf("rseed", (int) (mote.getSimulation().getRandomSeed() + newID));
			}
		}
		if (moteMem.variableExists("TOS_NODE_ID")) {
			moteMem.setIntValueOf("TOS_NODE_ID", moteID);
		}
		if (moteMem.variableExists("ActiveMessageAddressC__addr")) {
			moteMem.setIntValueOf("ActiveMessageAddressC__addr", newID);
		}
		if (moteMem.variableExists("ActiveMessageAddressC$addr")) {
			moteMem.setIntValueOf("ActiveMessageAddressC$addr", newID);
		}
		if (memoryMonitor == null) {
		    memoryMonitor = new MemoryMonitor.Adapter() {

		        @Override
		        public void notifyWriteAfter(int dstAddress, int data, Memory.AccessMode mode) {
		            byte[] id = new byte[2];
		            id[0] = (byte) (moteID & 0xff);
		            id[1] = (byte) ((moteID >> 8) & 0xff);
		            moteMem.setMemorySegment(dstAddress & ~1, id);
		        }

		    };

                    addMonitor("node_id", memoryMonitor);
                    addMonitor("TOS_NODE_ID", memoryMonitor);
                    addMonitor("ActiveMessageAddressC__addr", memoryMonitor);
                    addMonitor("ActiveMessageAddressC$addr", memoryMonitor);
		}

		notifyObservers();
	}

	public JPanel getInterfaceVisualizer() {
		JPanel panel = new JPanel();
		final JLabel idLabel = new JLabel();

		idLabel.setText("Mote ID: " + getMoteID());

		panel.add(idLabel);

		Observer observer;
		this.addObserver(observer = new Observer() {
			public void update(Observable obs, Object obj) {
				idLabel.setText("Mote ID: " + getMoteID());
			}
		});

		panel.putClientProperty("intf_obs", observer);

		return panel;
	}

	public void releaseInterfaceVisualizer(JPanel panel) {
		Observer observer = (Observer) panel.getClientProperty("intf_obs");
		if (observer == null) {
			logger.fatal("Error when releasing panel, observer is null");
			return;
		}

		this.deleteObserver(observer);
	}

	public void removed() {
	  super.removed();
	  if (memoryMonitor != null) {
	      removeMonitor("node_id", memoryMonitor);
	      removeMonitor("TOS_NODE_ID", memoryMonitor);
	      removeMonitor("ActiveMessageAddressC__addr", memoryMonitor);
	      removeMonitor("ActiveMessageAddressC$addr", memoryMonitor);
	      memoryMonitor = null;
	  }
	}

	private void addMonitor(String variable, MemoryMonitor monitor) {
	    if (moteMem.variableExists(variable)) {
	        int address = moteMem.getVariableAddress(variable);
	        if ((address & 1) != 0) {
	            // Variable can not be a word - must be a byte
	        } else {
	            mote.getCPU().addWatchPoint(address, monitor);
	            mote.getCPU().addWatchPoint(address + 1, monitor);
	        }
	    }
	}

        private void removeMonitor(String variable, MemoryMonitor monitor) {
            if (moteMem.variableExists(variable)) {
                int address = moteMem.getVariableAddress(variable);
                mote.getCPU().removeWatchPoint(address, monitor);
                mote.getCPU().removeWatchPoint(address + 1, monitor);
            }
        }
}