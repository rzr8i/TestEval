from faker import Faker
from random import randint

fake = Faker()

num_of_qs = 20

data = []

for i in range(0, 50):
    data.append({
        'name': fake.name(),
        'answers': [randint(0, 4) for i in range(0, num_of_qs)]
    })

for (i, d) in enumerate(data):
    with open(f"./data/file{str(i+1).zfill(2)}.txt", 'w') as f:
        f.write(d['name'] + '\n')
        for a in d['answers']:
            f.write(str(a)+'\n')

with open("./data/base.txt", "w") as f:
    for i in range(0, num_of_qs):
        f.write(str(randint(1, 4)))
        f.write('\n')
