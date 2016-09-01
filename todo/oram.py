

# bid : block id
# lid : leaf id

def read_block(block_id):
    lid = get_leaf_id(block_id)

    # traverse the tree
    for bucket_id in get_buckets_in_path(lid):
        addr = bucket_id_to_mem_address(bucket_id)

        # 1. read addr upto a bucket size
        # 2. decrypt each block, and store the matched block into safe
        # area - We can do Racoon here.
        # 3. write back

def write_block(block_id, block_data):
    lid = get_leaf_id(block_id)

    # traverse the tree
    for bucket_id in get_buckets_in_path(lid):
        addr = bucket_id_to_mem_address(bucket_id)

        # 1. read addr upto a bucket size
        # 2. decrypt each block, and store the matched block into safe
        # area - We can do Racoon here.
        # 3. write back


class VMEM:
    def oram_write(address, value):
        pass

    def oram_read(address)
        pass

def do_execute(inst, op1, op2, vregs, vmem):

    # Everytime, need to invoke the same number of oram access.

    mnemonic = get_mnemonic(inst)

    if mnemonic == "MOV":
        if is_mem_op(op1):
            # op1 - memory poitner
            # op2 - register
            # *op1 = op2
            vmem.oram_write(op1, vregs[op2])
        elif is_mem_op(op2):
            # op1 - register
            # op2 - memory pointer
            vregs[op1] = vmem.oram_read(op2)
        else:
            # op1 - register
            # op2 - register
            vregs[op1] = vregs[op2]

    elif mnemonic == "ADD":
        do_lea(op1, op2)

    # ...

def fetch_instruction(vmem, veip):
    return vmem.oram_read(veip)

def execution_loop():
    # virtual registers. Always stored in a real register file.
    vregs = {}
    vregs[eip] = initial_virtual_eip

    # virtual memory
    vmem = []

    while 1:
        inst = fetch_instruction(vmem, vregs[eip])
        op1 = get_operand(I, 1)
        op2 = get_operand(I, 2)

        do_execute(inst, op1, op2, vregs, vmem)
