dpctl del-flows

sh ovs-ofctl add-flow s1 "in_port=1 actions=output:2,4"
sh ovs-ofctl add-flow s1 "in_port=2 actions=output:4,1"
sh ovs-ofctl add-flow s1 "in_port=3 actions=drop"
sh ovs-ofctl add-flow s1 "in_port=4 actions=output:1,2"

sh ovs-ofctl add-flow s2 "in_port=1 actions=output:2,3,4"
sh ovs-ofctl add-flow s2 "in_port=2 actions=output:1,3,4"
sh ovs-ofctl add-flow s2 "in_port=3 actions=output:1,2,4"
sh ovs-ofctl add-flow s2 "in_port=4 actions=output:1,2,3"

sh ovs-ofctl add-flow s3 "in_port=1 actions=output:2,4"
sh ovs-ofctl add-flow s3 "in_port=2 actions=output:4,1"
sh ovs-ofctl add-flow s3 "in_port=3 actions=drop"
sh ovs-ofctl add-flow s3 "in_port=4 actions=output:1,2"






