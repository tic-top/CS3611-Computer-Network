from mininet.topo import Topo
from mininet.net import Mininet
from mininet.cli import CLI
from mininet.log import setLogLevel
from mininet.util import customClass
from mininet.node import OVSBridge

class Hw3( Topo ):

    def build( self ):
        "Create custom topo."
        s1 = self.addSwitch('s1')
        s2 = self.addSwitch('s2')
        s3 = self.addSwitch('s3')
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')
        h4 = self.addHost('h4')
        h5 = self.addHost('h5')
        h6 = self.addHost('h6')
        h3 = self.addHost('h3')
        self.addLink(h2, s1)
        self.addLink(h1, s3)
        self.addLink(h4, s1)
        self.addLink(h6, s2)
        self.addLink(s2, h5)
        self.addLink(s3, h3)
        self.addLink(s1, s3)
        self.addLink(s1, s2)
        self.addLink(s3, s2)

def con():
    topo = Hw3()
#    net = Mininet(topo=topo,switch=customClass({'ovsbr': OVSBridge},"ovsbr,stp=1"))
    net = Mininet(topo=topo)
    net.start()
    CLI(net)
    net.stop()

if __name__ == '__main__':
    setLogLevel('info')
    con()
