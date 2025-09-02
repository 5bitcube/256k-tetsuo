import solders

# HELPER FUNCTIONS #############################################################

def b58encode(byte_array):
    ALPHABET = '123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz'
    num = int.from_bytes(byte_array, 'big')
    leading_zeros = len(byte_array) - len(byte_array.lstrip(b'\x00'))
    result = []
    while num > 0:
        num, rem = divmod(num, 58)
        result.append(ALPHABET[rem])
    return ALPHABET[0] * leading_zeros + ''.join(reversed(result))

def b58decode(s):
    ALPHABET = '123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz'
    leading_zeros = 0
    for c in s:
        if c != ALPHABET[0]:
            break
        leading_zeros += 1
    num = 0
    for c in s[leading_zeros:]:
        num = num * 58 + ALPHABET.index(c)
    byte_length = (num.bit_length() + 7) // 8 if num > 0 else 0
    byte_array = num.to_bytes(byte_length, 'big')
    return b'\x00' * leading_zeros + byte_array    

################################################################################

full_key_user = "3Nyj9edXZGzvP1xX72AD4Zt47kq3EQmEF2cnZTZ2fRAatrpik7mVB5WxCA3m8JKYhTv6VZmyinSmAauCMDt9fvL9"
public_address_user = "7pLiHHtyDixRs5m2AACQpvhtmvEWoLRMw1ppRHQnqrLy"

def get_private_key(full_key):
    return b58encode(b58decode(full_key)[:32])
    
def get_public_address(private_key_base58):
    private_key_bytes = b58decode(private_key_base58)
    keypair = solders.keypair.Keypair.from_seed(private_key_bytes)
    return keypair.pubkey().__str__().split()[-1]

def private_key_to_bytes(private_key_base58):
    byte_array = b58decode(private_key_base58)
    return ' '.join(f'{b:02x}' for b in byte_array)

def bytes_to_private_key(byte_string):
    byte_array = bytes(int(b, 16) for b in byte_string.split())
    return b58encode(byte_array)

print(f"Solana keys verification tool. \n")
print(f"Full Key (user): {full_key_user}\n")

private_key = get_private_key(full_key_user)
print(f"Private Key (from full_key): {private_key}\n")

public_address = get_public_address(private_key)
print(f"Public Address (user): {public_address_user}")
print(f"Public Address (calc): {public_address}\n")

bytes_string = private_key_to_bytes(private_key)
print(f"Bytes String: {bytes_string}\n")

print(f"Private Key (from full_key): {private_key}")
private_key2 = bytes_to_private_key(bytes_string)
print(f"Private Key (from bytes_st): {private_key2}")