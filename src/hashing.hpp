#include "piece.hpp"
#include "square.hpp"
#include <cstdint>

static constexpr Squares::Array<Pieces::Array<Sides::Array<uint64_t>>>
    square_rands{0x94c2ef8841145504, 0x74c9924838face5d, 0x24b952a22bd51f44,
                 0xfa9619fb175542f,  0x253cb2404da1d178, 0xd2b3a4004d467811,
                 0x30d6f50bc9201bb4, 0xc7e7b253a34870a2, 0x1df6642e6b962812,
                 0xc8fce0b057e12c24, 0x5112cdac417cd1a1, 0xddf19a389e7e21ef,
                 0xd795884cb92cb63d, 0x1b219629007e6f9f, 0x5ac7977d89d81878,
                 0x45145be34c00eca1, 0xc0be4a836474266a, 0x7101b8f439ed1e2e,
                 0xa4594a6f3ff394f8, 0x9548babb5d3bdb59, 0x91800e9d5a446b42,
                 0xf63ea4b813a1fea1, 0x38742f86845b4c59, 0x1c92049510310e3c,
                 0x1ac5f8d1451dbb04, 0xde0573e820530508, 0x5862732b1e8eafd4,
                 0xa2610ce053a2adc5, 0x4b26e7645bb05cc7, 0x24f46fee0aa3385d,
                 0x8f544500a6c6b6a8, 0x1ca09f0f8af87e5,  0x6a98c2e1f87b2cc2,
                 0xb02201b94665d05e, 0x54a7c338705faad5, 0x34cbbf6490d3a9b,
                 0x2a08485f60c7adb3, 0xa3e070583f0006d6, 0x6de4a19d84344b99,
                 0x91eb75ff0bc1635d, 0xda33f8c9273164b2, 0xf7acc1e2b4506ab7,
                 0x335f8026511a87d7, 0x1f733237fc94683c, 0xb2b6913332d312a0,
                 0x5baef39d8e5ccd2f, 0x163de627db6b25be, 0x4b80a8b6564895a6,
                 0xf785774d86827550, 0x4230d82b2ea0ec30, 0xc6f0453259d8ac28,
                 0xba327a639b9bcd5a, 0xfc2a2bd836b9696b, 0x52479fbfc274b15a,
                 0x453a761ea8b5fccb, 0x5aad28fbbaec2096, 0xefd65d47924f42dd,
                 0x9402bad751ce8363, 0x409a7ccda6e82536, 0xc2b611b52db55481,
                 0x2bb0bae4fcd1cc54, 0x2393d3cb6b773c50, 0xbdbd9e477ed474e9,
                 0xa2e89aefc3e698d3, 0xa00006737f04783e, 0xbf876603e032ad2c,
                 0x70366c9da6bc9761, 0xca25955e254caa80, 0xaa57c58086eee11a,
                 0xd996bd08ef1496a9, 0xcd98ff950f5fc4d4, 0xbe5514453c54b74b,
                 0x5893257835ccb145, 0x425ff2a9b84f7a6e, 0x71be977b0fffe1e0,
                 0xf27c8183b7300e3e, 0xcd3c379db112e86,  0xd0a7c229ff8c3bbf,
                 0x9a8ba52bbe439161, 0x6b5bd17976c7e972, 0x5767b685a42dea12,
                 0xcbeb31d9a241e42f, 0xa61e585ae6a47381, 0x48e70bc4ed03f795,
                 0xa442ac0de7a70ab5, 0xf6eac82de1f05db5, 0xf38ce7f883b85ac5,
                 0x51914ded3c43d718, 0x804b3bcd3a8077a2, 0xe675ed60b9abaab1,
                 0xe6cc73ec31e38e44, 0x8398a0a81ee34f18, 0xd4ecb54183320332,
                 0x50fb15c18f530c9f, 0x229ad16d5ea741ea, 0x7f0f5bb363da3395,
                 0x439d3c5672f066da, 0x5618d84fbdf318a5, 0xea6e724b9fb33179,
                 0x82168dd26be06c94, 0x52a1a274cc42898f, 0x182007f8568778ef,
                 0xb6cfd75d693cd875, 0x9c8ae8ba48f1bd3f, 0xeb3aeaa2e4f352bc,
                 0xe6bd5818a233a51b, 0x5a8c303e247b1cc6, 0x4166418f32d0deea,
                 0x86adf3d6367dfd2b, 0x8a0bbdddcdc939bd, 0xaf4e369951e410d9,
                 0x9c44e5aeb8955138, 0xbf1c56989fad80c4, 0x324872407b06281b,
                 0x6e72e97bfc171954, 0xb96789185aa58d68, 0xef300a108fb2e490,
                 0xfc8b1a097893a92e, 0x1c6ad3a0f2fe1343, 0x7c89b18891911864,
                 0x21ae40cc9af1f90e, 0x63e531c70b1795d7, 0x665426fd44e393af,
                 0x874e2edb7baabaac, 0x39de2a2c5612137c, 0x57693117fac2f6d3,
                 0xeb08cf0afce5a236, 0xc1761760aa62d450, 0xb068da0418236cb0,
                 0x4e49d6cdd019ea25, 0xb44601e83feb08b8, 0xf4b18d807f1e6e0d,
                 0x7b5eb9bac402f476, 0x9d00a2e0b74c2a97, 0x8a84dfb659033d6b,
                 0x460aadae5107a352, 0xe94102dce4a217c7, 0x4fa9402185e99c20,
                 0xc7f10f88f34c8f40, 0xc366965de9074b1d, 0xe8ad43724e966148,
                 0xca86d01330008d94, 0xd0ae147ea4bd3120, 0x44bb32cba7b57ba0,
                 0x622160c1aafee47f, 0xcd5087145e53338c, 0x4873d64e2de9e7dd,
                 0xc9965da91cc6d780, 0x5cfe6d027df55803, 0x757ff10885f4885a,
                 0x219da7ef75dc6984, 0x6cd32c08d2ac1b7d, 0x620060d1ae878fb1,
                 0x13b6e77415c931e4, 0xfea15bdc8266cd1d, 0x2b89b6c764c618f6,
                 0x5b6672649a61bab9, 0x8a0ad55bfcba1ee5, 0x42ad92cf01dd8325,
                 0x1bcbc2a4c79c68bb, 0x85896ba613292ed1, 0xad7ea0bf6593868f,
                 0xf55c7e9fa2c80713, 0xb7599f731e5b2a0,  0xd060a056515e2feb,
                 0x117c70e659d892fb, 0x91b4fc803b1f0209, 0x87b03559f99810ba,
                 0x35f8512a588d2ec3, 0xad3e3fb26e98b5bc, 0x877b6b8569a175db,
                 0xdecb3b124fe57deb, 0x1a965f49382a0ffd, 0x418081b9fe7a521d,
                 0x30ca2d148add6a1d, 0x76bce15a3ef2f0ce, 0x600228bf9d8b6a20,
                 0xc824f69041503ad2, 0xbb1466203991f12e, 0xbfbcffca7bccc11a,
                 0x56101e5d46553a34, 0xdec3159998f23cee, 0xa9c9d403c2985580,
                 0xc968e77d223bf337, 0xa0b5ae8eae32d6e3, 0xb54c4822edf3d505,
                 0x30c91d97d29c3d4e, 0xc7277cbfe4eb3aa5, 0x2832830335b19244,
                 0xf545034616f998c8, 0x157220ad7aa7e2c8, 0xca47b449a4a31842,
                 0xbbd8020966e52282, 0xdd4b8b1bf14f62e1, 0xae8bfd37f4b87e30,
                 0xcfb352c04f9614f9, 0xcde55f9b475f7294, 0x3c203f2f360a50f0,
                 0xc4e45b09ab660fcb, 0x46483369ad5f03ca, 0x54cea9448a51d464,
                 0xaa8aaa2de3da6ea1, 0x533a4adca31b8409, 0x23306c79dad36cb,
                 0x2592b84aaadd9c6,  0x348ac18ecc95312e, 0xe2ec99c4a517c4e8,
                 0xc13e633f400326c,  0x311c10f94c88ef3f, 0xa910ebeaa09cda5d,
                 0x87e1cc4f84337732, 0x6d5a35c719dc1be6, 0xee8dc16bfa7cfcd8,
                 0xea281f02626ddb4a, 0xce7bef67af847c7c, 0x5f7d4afc7daaced5,
                 0xd36b49ba697e7af8, 0x53631f91e1adc76a, 0x48da36edd7ef2da4,
                 0x5671fdbff9ca0d2b, 0xe3ec7bb37f15ea14, 0x30c8b40feabc4265,
                 0x5dc88b2ce1865d38, 0xa983dde8291d3fc1, 0x37b861e3e77725a9,
                 0x9927cf7182f27f7b, 0x15dc874111428786, 0x7724be605e6c35ae,
                 0x1bafa8614ff57886, 0x802874134c2d6c1a, 0xcb0b100d4a031b7a,
                 0x323786ef446c70f,  0xfb57af0b09568bb1, 0xd8efe247b11748a7,
                 0x129d240963f6abb2, 0x5e28e89f76cc342b, 0xc5dfc1f7d58914ba,
                 0x6480461f23ad32f0, 0xc45bf1cdfc53cdd6, 0x5cb8581cfa91d458,
                 0xcec2331cceeeefcf, 0xcb29da0ec4cfe6a7, 0x2c73ef93c9fb0e2d,
                 0x5967806de3ed27bf, 0x3148799fce0b2e7a, 0xdf4194541a14f737,
                 0x973e1d6894418692, 0xfd350a20e14ffd30, 0xeca720458b433edc,
                 0xeea62591cdf641a3, 0x85778785b435c68b, 0x112b757b8f98470d,
                 0xce35e1497ea65cb0, 0xd48d7defb7f0ab9a, 0xbdea838316eb0c75,
                 0xa4018be6416b5a55, 0x38b20b828b253447, 0x91a630898d94df7e,
                 0x7b5a0a5ab88eb5c4, 0xc498ea14f4e0fab7, 0x4c2031289bcc776d,
                 0x5294d00774724d98, 0xb8c0f0d7f19b23e7, 0x3d465f96ad058d2d,
                 0x6de2664a6ff84f73, 0x9842b5a4937422c6, 0x7cf9768192d65b09,
                 0x319e15ff8d0ccc15, 0x2ec138b591e287ed, 0x2c8f0dca354f7a1e,
                 0x70caf4d3320598dc, 0xed30dcb25d2b1443, 0x9c12db6ee70378d1,
                 0x604aefd114f989c8, 0xde6b3c74d4c7110a, 0x50cf671dd1798ffb,
                 0x275d0d8973ee6d94, 0xdd82640a855eb67e, 0xdffa83a830d4dc9e,
                 0x433e52422ceb588c, 0xa7a3690b4f9e17ff, 0x191908bb736875d1,
                 0x212b08ea0bd5fbe6, 0x79ecf75b5bd0ec7f, 0x7196fbf8a9c15c00,
                 0xa83e664c922fa1b4, 0xfa45c6ab47a4445,  0x5d2e56ea33c3235b,
                 0xe33b88c3ca1f9c91, 0x3c52d8496319f2c,  0x97dce5e319957cc3,
                 0x241a6f618aefe4a4, 0x6166a576cdecb9b9, 0x467a68d190c13a4c,
                 0xe99c38b696e7db9a, 0x71997e5e85649bba, 0xa76f36a0a712c4de,
                 0x9af7e2535f79ceaf, 0x3175239dae75c16d, 0x3ba639e131109a19,
                 0x4bb6a297531ab534, 0x4b7de20631c6dfd7, 0x806295576132c97e,
                 0xc0d660a6e7d0fcde, 0x938520dc76d11518, 0x4cd6900605e7c530,
                 0xc013cc2efd620883, 0xd7161a19078fb301, 0xa17ab6967b0adb86,
                 0xd5dbdcfb95d19e4e, 0x588ee5f3f40f4150, 0xa27ed8982677766,
                 0x862b0cd5e3eacd,   0x2c7eea07d7182c81, 0xf43dd00dc3b8c714,
                 0xff5070a8c2ad4096, 0xdf06466d88b0bc2e, 0xffaa44f630a89a54,
                 0xa936795d4ccc8ed3, 0x38687848b52063af, 0x661d950cec8ac175,
                 0xaa41f06f954fbb4,  0x459d3d54c3ab2c10, 0x1c3ad138eb78fca6,
                 0x347de91a5b5d498c, 0x2a583dda85839853, 0x6147f0bc207c6c79,
                 0xe1547f646e39a10e, 0x8c8383d24c304389, 0x85ff9a57be5654dd,
                 0xcea05296e6421c00, 0xb22997c311ede2fb, 0x1f9912e487a4162b,
                 0x89fd593d16cab098, 0x5b24cdde7b99ebf2, 0x32da261fe1f13c9b,
                 0x1a9c509361005f66, 0x46f560bb5fc7d071, 0xf24d4a29ad565eb,
                 0x95443d4626ccc415, 0x27ba09963119eb9c, 0x7ba851520f490e95,
                 0x99067d7535502bc,  0x82375fa5b112ed1b, 0x19998b914b728922,
                 0xf3a2f71761ee1d92, 0x907d1758045b12fb, 0x2d50c227e2cb16b8,
                 0x703b0c3fa3017db,  0xd512b4c19f7f5482, 0x604f36bfc939d2a9,
                 0xd93df2c073e384a6, 0x81516a2f2a90de28, 0x3c32c886c7575a82,
                 0x6af68b259b29b00a, 0x551f0b26f12c46c6, 0x89d3d83a1de8dbc5,
                 0x304bcb8aaae451c0, 0xadf5173d1915260f, 0xbb85e9bf0836b7ae,
                 0x5541b5b71de66176, 0x66395fcaa8fbdb49, 0xe303fa565ce90986,
                 0x8232a9015b033541, 0x93d21d6155f8c50c, 0x9e38fa5ca45cd0ff,
                 0x23fe30fa89dee8d8, 0xd6f092bf157fc697, 0x23a38c2c40019c07,
                 0x6808c47e5c66196a, 0x7e4bf88fec5daf65, 0x1d4bff19c1d5b758,
                 0x5397628e8690a493, 0x6797e87c2d32d3c1, 0xf6488ce4507b160a,
                 0x90096f74ee01703d, 0xb24f5aca9cc0f7b4, 0x4d6a6c7b1b4f8928,
                 0xbe4ca4f8e4d4fdb,  0x2483d2435412ab86, 0x6ef6e5750e23709f,
                 0xdeabb98783dc0b4,  0xe7cf5b2186dea8ae, 0x58117d50036c42a9,
                 0x726a095aeed76824, 0x27e165ee49ff6543, 0x273c2ee5a5b6a97d,
                 0x1fa8ae852ff92930, 0xd61be069d7e0e796, 0x8c2db69accc18025,
                 0x8e41e95214a5cb32, 0x4a03aba87a81f525, 0x1d80ef65d44a4833,
                 0xd1cc170708181a24, 0x6fdd614b451dd9e7, 0x73c5723af7482576,
                 0x662d643abf02957a, 0x86bf5173af13cb51, 0x50d110ddd02e3b63,
                 0xa6d5cb708f8ef052, 0xcdd4cad1bb77c96d, 0xd090d75a4e7676f6,
                 0x2a6bf6948e1ef4c4, 0x121ce078ad83ebef, 0xef09ee264897b33,
                 0x92cbd87b0feaba62, 0x1e20c73b29f8fccb, 0x91584b5f87f6bfa2,
                 0xf9504a4a4b47a955, 0x98f93579f082858e, 0x2fd3a5c6cfa9c3c6,
                 0x940fbca05b48b2b9, 0x5df6250ef47d4c38, 0xb1533c9edfd6524d,
                 0x873d0f55f827fac8, 0xbc61aac201779af8, 0x8852556309ab85b0,
                 0x6ed6532ec5276afd, 0x1f9acaa2dde6c6c4, 0x9be90530e541e7d,
                 0x61bff22159d64c3c, 0x7b44ca73928bd67b, 0xbedf4b1acc706bd9,
                 0x6d8680137176b99a, 0x54ab5ed905fdc97e, 0x2e361e8c4c503288,
                 0xd822a7a18338b8fc, 0xdfc065db10227aa2, 0xd272ef6ee1c0ab20,
                 0xde30227ec1ca1848, 0x6734dbdbe90beeed, 0x6f2474c2cbacaf18,
                 0xa463ac92c540fea4, 0x83f5545160735966, 0x4fa129b099339318,
                 0xb83177eec304366c, 0xd209b5251fe782e3, 0x929d2db44970c246,
                 0xb8244e3b718acdd7, 0xf4986aee52138042, 0x89c07d6e6c99adc6,
                 0x8dd79894cae5f457, 0x431d169da8363f36, 0x99d648a869c809a6,
                 0x2fa74974df03e795, 0x34d0ca0f347145ea, 0x8543df3479836e6b,
                 0xea4038859021d5e7, 0x79698aecd45f8889, 0x61a350eaa968b4de,
                 0x189b3ebc2e08865b, 0xfe88da9c5f8e2a9c, 0x2cab0419a70a14d8,
                 0x187e63557efd9f73, 0x87fbd0c3178687c7, 0xc1aa09ccca9ad253,
                 0xc0bc463079ec0f6b, 0x417df9d796199ef8, 0xfffd96b371358359,
                 0xf4bb3a40af87ae4a, 0xa057aa11e5f8dba8, 0x76508c3dcf0f861d,
                 0x543d6bc5791a92be, 0xb86c9ea46d79451d, 0x2165daaeb15e2442,
                 0x8f28ec8899481e63, 0x4d888862bf87e716, 0x7cd4db802e39adc3,
                 0x5a3ce45e9f8d576b, 0xbcb2188735555723, 0x4d7790423a93c85,
                 0xbb1f2328f8a0b5c1, 0xfe0b6988c0834419, 0xdde23ac8ca6d7065,
                 0x984e3005b45a7d29, 0xb9206ab07f7530ef, 0x7b18795ad55d2,
                 0x23bbc9f29d95e303, 0xd6533f61e05d72f1, 0xee8883c2cc8ec93d,
                 0x5b566abfc2c5bf60, 0x28d0adaaddb930f6, 0xe26dece5cda5ddc,
                 0xe27bac581a9eac70, 0xb30769509970f4c4, 0x4d33fe8127d0fd8e,
                 0x8462f123294d4ea,  0x7f020d4a9f1db3cf, 0x947d4dd2372bd386,
                 0x4a457b7e68433b26, 0x8abb2b77029dabc9, 0x43e94b4d980cbffa,
                 0x1c0c067f05c5d947, 0x4b9cff542e88ae96, 0xae97fd7a992720ea,
                 0xde01b939f075798c, 0xa907914dc0d1f8bb, 0x223e9b1082ac8f96,
                 0x341027e64734ed5b, 0xbc8a995b4a4035f5, 0x90ff052cda75aa32,
                 0x3277ad0acafd32,   0xd0e0113318add9f0, 0xe7298618cbf44e88,
                 0x470910c17e89cff1, 0x19502959601aa58f, 0xa5b2120c697581f5,
                 0xb032d1f2eb3fab38, 0xf04cc32643431ef1, 0x4c28f99ffa2b3434,
                 0xc28b094e1ea82d94, 0x65ca3f9185868cb7, 0x59605f6ea7d43082,
                 0xdaafef072589e429, 0x72d3adcb9c66f977, 0x45cdd6d272ded321,
                 0x90341ac61dc94973, 0x6d2c061f1900630a, 0x3308ce0807ff73ae,
                 0x566f30384eba9bd4, 0xec5ead382d37f312, 0x7927fda82301829c,
                 0x1fac50dffd162aeb, 0x5b7e52a7c241c60a, 0x7ca0109a0393dac1,
                 0xe62af33c2be27e98, 0xfceb51f68a95f9a5, 0x4692bcfcfb29e148,
                 0xc77fa8dd285f45b5, 0x69715c54e90bd8a8, 0x62537c81b357c001,
                 0x94dd36a470134a1e, 0x492c95903317cd90, 0xc63b62475b34e36b,
                 0xcc6eb4ada26402fa, 0x53a1b4580b2dcdc8, 0x24d44b1383b44b7d,
                 0x75283ad4457ab8fd, 0x11857289251bbfab, 0x912f3cd2c8cbdcbc,
                 0x9e8565e6e3fd4b33, 0xef007287f78e3d4b, 0x55f8a35d89a95b04,
                 0xd513193b96534e37, 0x91839acd2e3c6997, 0xc4a90234d8d94d4f,
                 0xb871e736b8f79c4e, 0x2b29b62c7e7f1b8,  0xe4f8f08965db7d4e,
                 0x176f7d4e29e6d752, 0x67c669ff686a1c2,  0x1f5420e757ed8819,
                 0xd837749a50fd4703, 0x5ede4bdc50b79074, 0xc80938e216bc800b,
                 0x6b110c61f1c16b3d, 0xfbbb752c78648d73, 0xdc1c8dcfdbbe04d8,
                 0x67c152f87eb78a6a, 0x5eefda22a91d080c, 0x417b2285ad7cb72,
                 0xb3d9f77e7704c7a,  0x407f2fae1381a0cb, 0xe308eee03abe5e7a,
                 0xac51d9649c3d751e, 0x8040d58eb517cf26, 0x99164b0ebd22aea0,
                 0xcf503d3bcab6d4bc, 0x5d6bcf45d983785f, 0x3f38fc3519c1b733,
                 0x532b45e997fd9692, 0x37a234d58baa3e4e, 0xbe14d9572fc80d2a,
                 0x2dea729c1d4b49db, 0xa3dab7eaa8fb6f11, 0x981ee978c90d7719,
                 0x21b2144a0042b1b1, 0xe497af2123a31c7e, 0x129aef68d0f702d2,
                 0x6dd89b1f8e2ce00,  0x722b29bce10d94bd, 0x400a10f623786ff5,
                 0x7a27e4c6b1c1399f, 0xc9c4d7476bf2c8b1, 0xf0170179ded7d4f6,
                 0x904c703b7fb53464, 0x95d8a5053604d8cc, 0x72ebb214708571c0,
                 0x7d192f4117009172, 0x3c66867d01e45f6d, 0x8970c388ef665479,
                 0x446c41718c01d6e0, 0xe2b8ffc0ae09734c, 0xddd0633efb1660e5,
                 0x46b9def82692dc57, 0x370942cb39495f1d, 0x6bc212b822a430f8,
                 0xa9e1289de5d38000, 0xfbff3576e0e182db, 0x500f21bde97f8ff2,
                 0xb3f66d7a695dad19, 0xf40d4a1c3464c173, 0x7a97f6fbedf6ab02,
                 0x317b8d6abd71428d, 0xcf118ed031da148c, 0x88b8727283ae1e7c,
                 0x4f39bf7b1bb67b4e, 0x9867194d5cae0091, 0xce02229253fb2fbe,
                 0xb8ee474a23b9853a, 0xb2856594af76a286, 0x485277e82dc8e757,
                 0xa79d33141b374d8,  0x9c73ea4d593f1cba, 0xaa3aa883e8a97894,
                 0xe9fc764f7ff55c8e, 0x6a03ee02e5afab3,  0xbf24c99b1ac7814e,
                 0xbef47d2eb19fea03, 0x9401cbca8601ecf4, 0x2bb26cdaff741351,
                 0xc3a386f01b0d953c, 0xe1adddf90fa2f444, 0xf61ca1c58f7bcf52,
                 0x197909ce15dc68f9, 0xa0034f496eb32e27, 0x32a5bf571f567ce2,
                 0x83bdea60e2f998ca, 0x972c0a828b403fad, 0x20f3223d67bf899a,
                 0x3d2846a5c41c7628, 0xe38d0d2e455b158d, 0xbaffb403614596ab,
                 0x37a2ca355bf92eaf, 0x5730141f5a98c758, 0xfdfceacc5233b77d,
                 0x1cd71358442e4ad7, 0xa3a2f5062be0c7b6, 0xb6a1ddcc2826fdbc,
                 0x2dc48d48e262e611, 0x4c7529bc8eeb6078, 0xe5ba9fe4cbc01e52,
                 0xbc3ada5827072b5b, 0xcc414dfc4a034451, 0x33471688fa649ce2,
                 0x7ace5bf7c38716f0, 0x89b6d05d883aa35c, 0x6f3960e84ffa34ea,
                 0x274f316628db038e, 0xac26eefe243c7753, 0xad71e39645ad2342,
                 0xf43adb86139272ea, 0xcc9b8e55d6de0142, 0x9de6a9a2f44a56d,
                 0x510ae693bd7184f1, 0xb5f6b60d465254c0, 0xd3c46cd11032e714,
                 0x58e14ca765712a09, 0x4863a59eb64d8e68, 0x6b37162d60e48e0e,
                 0x680aa42680756022, 0x42cff4d0adfd573e, 0xda2604a0d8d98c06,
                 0x63197468ae7ed3d2, 0xb385fc96e1d03b6e, 0xbd181f5bec20cbed,
                 0xee67ba689d353721, 0x59b72f8768e0d4f,  0x40d064ebb2b34921,
                 0x231502186e06a722, 0xbb7b507001e0a306, 0xb0249d24ebb762d2,
                 0x68222b18a92514,   0xfe025dd1e91bab99, 0x76af947f52fc65eb,
                 0x4db6de025531cb4,  0x969ac7e7f325187c, 0x2dd427c1e0569753,
                 0x6b16a5be78f58640, 0xd88858f0dfaa9e82, 0x3f124d88ec9542dd,
                 0xd25112d798d57fae, 0x58a457fab581b9f9, 0xd255b0df8518582c,
                 0xc315bf08a209050,  0x52e42b1036089f92, 0xb9f74c69dd3fa37e,
                 0xbe8bdf2d1dc72996, 0x668ab4adcd114414, 0x684695c28fe8c27c,
                 0x18bcfe8cfcce97bc, 0xc4b21629948cba26, 0x58dcb4777228fb83,
                 0x9d8e598aed70999d, 0xf9e6da1400073753, 0xaa7ec9359aba4d09,
                 0xccbf4ab9dc321bf1, 0x4da24a53319d2a7b, 0x321c07cb265e52ac,
                 0x3128662e43b86bbe, 0x98191935b73eea49, 0xe120ea5a8dfeec82,
                 0xccf48cf1339f2c41, 0xaf5f0efdcfba6011, 0x1ba9c804c9ed8ad7,
                 0xe5ef8a1ee637e67c, 0x76840ef3ad4f8d18, 0x9355d2428f86d84,
                 0x88f43c015e8da6f8, 0x18d1837e2cc9f556, 0x3f0f3b75e1233993,
                 0x583cff40beb4069a, 0xbe32d02970ad59ad, 0x850a9ebd008de7ce,
                 0x59dfa54ca228f98d, 0xef7f412034eb2f2f, 0x8303fc636137bc46,
                 0xe0bdc457f6952c3f, 0xb9f02718691aab6e, 0x240b00646ab253c8,
                 0xc4cee76921f0e0fa, 0x4704f07b7d3203b4, 0xf093a24fbd1836bf,
                 0x83636e9c9203c65,  0xc6b2ea7fa62527c,  0x14922ea6bc47a005,
                 0xc6fb0869c96ea88f, 0x70a2e8082b869975, 0xf44ab6b5467283da,
                 0x6ba9db1ad5271ce1, 0xcef5d70cf7f72dee, 0xf37a7fa205fcd4dc,
                 0x2124b39845dae033, 0xf6f81d80b05d5442, 0x1538f82c2f69ea41,
                 0xa0c9145ee188a1e7, 0x8945e5df727347b,  0x45e6649c0c5096ba,
                 0x6d765f112abf79e3, 0x9812ffbcfeb5a26,  0xf3a60eeb5e0e1b39,
                 0x64b972fa8073f34f, 0x5fc83920245ade3a, 0xfb43a37ed65cf3ad,
                 0xc0cff1752faaf801, 0x217d0c0437e9ff5a, 0x7b9e17fd2b38794,
                 0x18dc14f9a1169d9b, 0x40ad4e4a39328a06, 0xc44eb310ddef893f,
                 0xec58298fcc7a921f, 0xbb608660c636b27b, 0x607c6e63997ca,
                 0x80cee48e4316f13f, 0xc4fbb1e06430cb6f, 0x7e4089b316dad538,
                 0xdebea452155f8a08, 0xae8a061b51dd02ad, 0x120fc786925b0caa,
                 0xda405b4ab769cdcc, 0x3552f4efeef1807a, 0x7f74421a57b7039e,
                 0xdddefd83070787da, 0x403104f927a7e788, 0x4ac8ea18b0cf1609,
                 0x50a20c9b2ac57db1, 0x2b8770913539b85d, 0xcf496c1388ad7e84};

static constexpr uint64_t stm_rand = 0x469a8453908236f;

static constexpr Sides::Array<std::array<uint64_t, 2>> castling_rands{
    0x24700a5fb94e062e, 0xc2e28bc9a5335e0e, 0x388e41bc9cee55c6,
    0x72bcd3908568bb34};

static constexpr std::array<uint64_t, 8> ep_rands{
    0x51386ecce05335b9, 0x2b076463851c7805, 0xe820b0a867247c8f,
    0xb006d52145f897f3, 0x9ea4d0bc8d58ad78, 0xd503bfa9e1c20de6,
    0x6d2daf158ca8ffb9, 0x2490fd654666fd30};
