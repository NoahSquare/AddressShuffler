

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


def execution_loop():
    # virtual registers. Always stored in a real register file.
    virtual = {}
    while 1:
        inst = fetch_instruction(virtual[eip])
        op1 = fetch_op(I, 1)
        op2 = fetch_op(I, 2)

        do_execute(inst, op1, op2)
