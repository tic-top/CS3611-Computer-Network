from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.log import setLogLevel

class Hw2( Topo ):

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
        s1s3 = {'loss':10}
        self.addLink(s1, s3, **s1s3)
        s1s2 = {'loss':10}
        self.addLink(s1, s2, **s1s2)

def iperftest():
    topo = Hw2()
    net = Mininet(topo=topo,
                  host=CPULimitedHost, link=TCLink,
                  autoStaticArp=True)
    net.start()
    h1,h2,h3,h4,h5,h6 = net.getNodeByName('h1','h2','h3','h4','h5','h6')
    for pair in [(h1,h2),(h1,h3),(h1,h4),(h1,h5),(h1,h6)]:
        net.iperf(pair)
    net.stop()

if __name__ == '__main__':
    setLogLevel('info')
    iperftest()
