/* lex -a -p CMD -o code\scan.cpp code\scan.l */
#define YYNEWLINE 10
#define INITIAL 0
#define CMD_endst 148
#define CMD_nxtmax 2349
#define YY_LA_SIZE 19

static unsigned int CMD_la_act[] = {
 0u, 72u, 37u, 72u, 2u, 72u, 3u, 72u, 4u, 72u, 6u, 72u, 52u, 72u, 42u, 72u,
 39u, 72u, 38u, 72u, 48u, 72u, 40u, 72u, 43u, 72u, 35u, 72u, 34u, 72u, 72u, 36u,
 72u, 49u, 72u, 50u, 72u, 29u, 72u, 30u, 72u, 31u, 72u, 32u, 72u, 33u, 72u, 41u,
 72u, 44u, 72u, 45u, 72u, 46u, 72u, 47u, 72u, 51u, 72u, 67u, 72u, 67u, 72u, 67u,
 72u, 67u, 72u, 67u, 72u, 67u, 72u, 67u, 72u, 67u, 72u, 67u, 72u, 67u, 72u, 67u,
 72u, 69u, 72u, 69u, 72u, 72u, 69u, 70u, 70u, 68u, 67u, 67u, 66u, 67u, 67u, 67u,
 67u, 67u, 67u, 67u, 67u, 63u, 67u, 61u, 67u, 60u, 67u, 67u, 67u, 67u, 67u, 67u,
 64u, 67u, 67u, 67u, 67u, 59u, 67u, 67u, 67u, 67u, 67u, 58u, 67u, 67u, 67u, 57u,
 67u, 67u, 67u, 67u, 67u, 56u, 67u, 67u, 67u, 67u, 67u, 67u, 67u, 67u, 62u, 67u,
 67u, 67u, 55u, 67u, 67u, 54u, 67u, 67u, 67u, 67u, 53u, 67u, 25u, 23u, 65u, 71u,
 71u, 65u, 21u, 16u, 15u, 19u, 14u, 20u, 13u, 12u, 26u, 11u, 24u, 10u, 17u, 27u,
 9u, 18u, 28u, 8u, 7u, 6u, 6u, 6u, 6u, 6u, 6u, 5u, 2u, 1u, 22u, 1u,
 0u, 0u
};

static unsigned char CMD_look[] = {
 0
};

static int CMD_final[] = {
 0u, 0u, 2u, 4u, 6u, 8u, 9u, 10u, 12u, 14u, 16u, 18u, 20u, 22u, 24u, 26u,
 28u, 30u, 31u, 33u, 35u, 37u, 39u, 41u, 43u, 45u, 47u, 49u, 51u, 53u, 55u, 57u,
 59u, 61u, 63u, 65u, 67u, 69u, 71u, 73u, 75u, 77u, 79u, 81u, 83u, 85u, 86u, 87u,
 87u, 87u, 87u, 88u, 89u, 89u, 90u, 91u, 92u, 94u, 95u, 96u, 97u, 98u, 99u, 100u,
 101u, 103u, 105u, 107u, 108u, 109u, 110u, 111u, 112u, 114u, 115u, 116u, 117u, 119u, 120u, 121u,
 122u, 123u, 125u, 126u, 127u, 129u, 130u, 131u, 132u, 133u, 135u, 136u, 137u, 138u, 139u, 140u,
 141u, 142u, 144u, 145u, 146u, 148u, 149u, 151u, 152u, 153u, 154u, 156u, 157u, 158u, 159u, 159u,
 160u, 161u, 162u, 163u, 164u, 165u, 166u, 167u, 168u, 169u, 170u, 171u, 172u, 173u, 174u, 175u,
 176u, 177u, 178u, 179u, 180u, 181u, 182u, 183u, 184u, 185u, 186u, 187u, 187u, 187u, 188u, 188u,
 189u, 190u, 191u, 192u, 193u
};
#ifndef CMD_state_t
#define CMD_state_t unsigned char
#endif

static CMD_state_t CMD_begin[] = {
 0u, 0u, 0
};

static CMD_state_t CMD_next[] = {
 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 1u, 5u, 1u, 1u, 4u, 45u, 45u,
 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u,
 1u, 9u, 6u, 3u, 17u, 19u, 12u, 7u, 24u, 25u, 18u, 16u, 30u, 15u, 26u, 2u,
 43u, 44u, 44u, 44u, 44u, 44u, 44u, 44u, 44u, 44u, 14u, 27u, 11u, 8u, 10u, 45u,
 21u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 41u, 42u, 42u, 42u, 42u, 42u, 42u,
 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 22u, 45u, 23u, 20u, 42u,
 45u, 42u, 32u, 34u, 42u, 36u, 40u, 37u, 42u, 39u, 42u, 42u, 42u, 42u, 33u, 42u,
 42u, 42u, 35u, 42u, 42u, 42u, 42u, 38u, 42u, 42u, 42u, 28u, 13u, 29u, 31u, 45u,
 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u,
 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u,
 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u,
 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u,
 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u,
 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u,
 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u,
 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u, 45u,
 47u, 55u, 46u, 46u, 46u, 46u, 46u, 46u, 46u, 46u, 46u, 46u, 148u, 148u, 148u, 148u,
 48u, 52u, 59u, 49u, 60u, 49u, 61u, 48u, 50u, 50u, 50u, 50u, 50u, 50u, 50u, 50u,
 50u, 50u, 51u, 51u, 51u, 51u, 51u, 51u, 51u, 51u, 51u, 51u, 57u, 62u, 63u, 64u,
 48u, 52u, 58u, 65u, 68u, 69u, 70u, 48u, 53u, 53u, 53u, 53u, 53u, 53u, 53u, 53u,
 53u, 53u, 71u, 72u, 73u, 74u, 75u, 66u, 76u, 53u, 53u, 53u, 53u, 53u, 53u, 67u,
 77u, 78u, 79u, 80u, 81u, 82u, 83u, 84u, 85u, 86u, 87u, 88u, 89u, 92u, 54u, 54u,
 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 53u, 53u, 53u, 53u, 53u, 53u, 54u,
 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u,
 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 90u, 93u, 94u, 91u, 54u, 95u, 54u,
 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u,
 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 54u, 56u, 56u, 56u, 56u, 56u, 56u, 56u,
 56u, 56u, 56u, 56u, 96u, 97u, 98u, 99u, 100u, 101u, 56u, 56u, 56u, 56u, 56u, 56u,
 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u,
 56u, 56u, 56u, 56u, 102u, 103u, 104u, 105u, 56u, 106u, 56u, 56u, 56u, 56u, 56u, 56u,
 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u, 56u,
 56u, 56u, 56u, 56u, 107u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 114u,
 120u, 127u, 108u, 122u, 126u, 125u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u,
 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u,
 128u, 129u, 130u, 131u, 109u, 132u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u,
 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u,
 148u, 148u, 121u, 148u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u,
 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 148u, 148u,
 148u, 148u, 111u, 148u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u,
 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 112u, 112u,
 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 148u, 148u, 148u, 148u, 148u, 148u, 112u,
 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u,
 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 148u, 148u, 148u, 148u, 112u, 148u, 112u,
 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u,
 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 112u, 113u, 113u, 113u, 113u, 113u, 113u, 113u,
 113u, 113u, 113u, 113u, 148u, 148u, 148u, 148u, 148u, 148u, 113u, 113u, 113u, 113u, 113u, 113u,
 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u,
 113u, 113u, 113u, 113u, 148u, 148u, 148u, 148u, 113u, 148u, 113u, 113u, 113u, 113u, 113u, 113u,
 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u, 113u,
 113u, 113u, 113u, 113u, 115u, 116u, 123u, 144u, 118u, 148u, 148u, 148u, 148u, 148u, 148u, 148u,
 148u, 147u, 148u, 147u, 147u, 145u, 148u, 117u, 119u, 148u, 148u, 148u, 148u, 124u, 133u, 133u,
 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 147u, 133u, 133u, 133u, 133u, 133u, 133u, 133u,
 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u,
 133u, 133u, 133u, 133u, 133u, 135u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u,
 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u,
 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u,
 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 134u, 133u, 133u, 133u, 133u, 133u,
 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u,
 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u,
 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u,
 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u,
 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u,
 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u,
 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u,
 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u,
 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u,
 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 133u, 136u, 136u,
 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 148u, 136u, 136u, 136u, 136u, 136u, 136u, 136u,
 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u,
 136u, 136u, 136u, 136u, 136u, 138u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u,
 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u,
 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u,
 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 137u, 136u, 136u, 136u, 136u, 136u,
 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u,
 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u,
 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u,
 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u,
 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u,
 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u,
 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u,
 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u,
 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u,
 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 136u, 140u, 140u,
 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 148u, 140u, 140u, 148u, 140u, 140u, 140u, 140u,
 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u,
 141u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u,
 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u,
 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u,
 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 139u, 140u, 140u, 140u, 140u, 140u,
 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u,
 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u,
 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u,
 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u,
 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u,
 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u,
 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u,
 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u,
 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u,
 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 140u, 142u, 142u,
 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 148u, 142u, 142u, 142u, 142u, 142u, 142u, 142u,
 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u,
 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u,
 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u,
 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u,
 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u,
 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u,
 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u,
 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u,
 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u,
 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u,
 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u,
 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u,
 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u,
 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u,
 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 142u, 143u, 143u,
 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 148u, 143u, 143u, 148u, 143u, 143u, 143u, 143u,
 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u,
 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u,
 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u,
 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u,
 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u,
 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u,
 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u,
 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u,
 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u,
 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u,
 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u,
 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u,
 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u,
 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u,
 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 143u, 146u, 146u,
 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 148u, 146u, 146u, 148u, 146u, 146u, 146u, 146u,
 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u,
 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u,
 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u,
 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u,
 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u,
 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u,
 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u,
 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u,
 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u,
 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u,
 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u,
 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u,
 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u,
 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u,
 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 146u, 0u
};

static CMD_state_t CMD_check[] = {
 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
 44u, 41u, 44u, 44u, 44u, 44u, 44u, 44u, 44u, 44u, 44u, 44u, 50u, 49u, 50u, 49u,
 51u, 43u, 58u, 48u, 59u, 48u, 60u, 44u, 48u, 48u, 48u, 48u, 48u, 48u, 48u, 48u,
 48u, 48u, 47u, 47u, 47u, 47u, 47u, 47u, 47u, 47u, 47u, 47u, 40u, 61u, 62u, 63u,
 51u, 43u, 40u, 57u, 67u, 68u, 69u, 44u, 52u, 52u, 52u, 52u, 52u, 52u, 52u, 52u,
 52u, 52u, 70u, 71u, 38u, 73u, 74u, 39u, 75u, 52u, 52u, 52u, 52u, 52u, 52u, 39u,
 37u, 77u, 78u, 79u, 80u, 36u, 82u, 83u, 35u, 85u, 86u, 87u, 88u, 91u, 42u, 42u,
 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 52u, 52u, 52u, 52u, 52u, 52u, 42u,
 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u,
 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 34u, 92u, 93u, 34u, 42u, 94u, 42u,
 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u,
 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 55u, 55u, 55u, 55u, 55u, 55u, 55u,
 55u, 55u, 55u, 55u, 95u, 96u, 90u, 98u, 99u, 33u, 55u, 55u, 55u, 55u, 55u, 55u,
 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u,
 55u, 55u, 55u, 55u, 101u, 32u, 103u, 104u, 55u, 105u, 55u, 55u, 55u, 55u, 55u, 55u,
 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u, 55u,
 55u, 55u, 55u, 55u, 20u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 18u,
 14u, 126u, 19u, 13u, 11u, 11u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u,
 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u,
 10u, 10u, 129u, 9u, 19u, 8u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u,
 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u, 19u,
 110u, 255u, 13u, 255u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u,
 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 255u, 255u,
 255u, 255u, 110u, 255u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u,
 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 110u, 111u, 111u,
 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 255u, 255u, 255u, 255u, 255u, 255u, 111u,
 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u,
 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 255u, 255u, 255u, 255u, 111u, 255u, 111u,
 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u,
 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 111u, 109u, 109u, 109u, 109u, 109u, 109u, 109u,
 109u, 109u, 109u, 109u, 255u, 255u, 255u, 255u, 255u, 255u, 109u, 109u, 109u, 109u, 109u, 109u,
 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u,
 109u, 109u, 109u, 109u, 255u, 255u, 255u, 255u, 109u, 255u, 109u, 109u, 109u, 109u, 109u, 109u,
 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u, 109u,
 109u, 109u, 109u, 109u, 17u, 16u, 12u, 2u, 15u, 255u, 255u, 255u, 255u, 255u, 255u, 255u,
 255u, 1u, 255u, 1u, 1u, 2u, 17u, 16u, 15u, 255u, 255u, 255u, 255u, 12u, 7u, 7u,
 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 1u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u, 134u, 134u,
 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 255u, 134u, 134u, 134u, 134u, 134u, 134u, 134u,
 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u,
 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u,
 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u,
 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u,
 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u,
 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u,
 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u,
 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u,
 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u,
 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u,
 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u,
 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u,
 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u,
 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u,
 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 134u, 6u, 6u,
 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 255u, 6u, 6u, 255u, 6u, 6u, 6u, 6u,
 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u, 139u, 139u,
 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 255u, 139u, 139u, 139u, 139u, 139u, 139u, 139u,
 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u,
 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u,
 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u,
 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u,
 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u,
 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u,
 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u,
 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u,
 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u,
 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u,
 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u,
 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u,
 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u,
 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u,
 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 139u, 3u, 3u,
 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 255u, 3u, 3u, 255u, 3u, 3u, 3u, 3u,
 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u,
 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u,
 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u,
 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u,
 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u,
 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u,
 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u,
 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u,
 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u,
 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u,
 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u,
 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u,
 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u,
 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u,
 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 3u, 144u, 144u,
 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 255u, 144u, 144u, 255u, 144u, 144u, 144u, 144u,
 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u,
 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u,
 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u,
 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u,
 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u,
 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u,
 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u,
 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u,
 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u,
 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u,
 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u,
 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u,
 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u,
 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u,
 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 144u, 0u
};

static CMD_state_t CMD_default[] = {
 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u,
 148u, 19u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u,
 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 148u, 44u, 148u, 148u, 44u, 148u,
 148u, 48u, 48u, 47u, 148u, 52u, 42u, 148u, 55u, 42u, 42u, 42u, 42u, 42u, 42u, 42u,
 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u,
 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u,
 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 42u, 148u, 148u, 148u, 19u, 148u,
 111u, 109u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u, 148u,
 148u, 148u, 148u, 148u, 148u, 7u, 148u, 148u, 7u, 134u, 7u, 148u, 6u, 148u, 6u, 3u,
 148u, 148u, 144u, 1u, 0u
};

static int CMD_base[] = {
 0u, 792u, 744u, 1838u, 2350u, 2350u, 1326u, 814u, 488u, 486u, 483u, 456u, 752u, 454u, 454u, 747u,
 746u, 745u, 450u, 453u, 439u, 2350u, 2350u, 2350u, 2350u, 2350u, 2350u, 2350u, 2350u, 2350u, 2350u, 2350u,
 355u, 336u, 285u, 243u, 233u, 228u, 220u, 225u, 189u, 189u, 302u, 185u, 210u, 2350u, 2350u, 242u,
 232u, 226u, 225u, 203u, 264u, 2350u, 2350u, 377u, 2350u, 193u, 164u, 177u, 162u, 196u, 191u, 193u,
 2350u, 2350u, 2350u, 193u, 193u, 213u, 212u, 207u, 2350u, 220u, 218u, 227u, 2350u, 226u, 240u, 242u,
 232u, 2350u, 227u, 242u, 2350u, 229u, 229u, 233u, 238u, 2350u, 341u, 239u, 278u, 290u, 288u, 319u,
 336u, 2350u, 324u, 325u, 2350u, 360u, 2350u, 369u, 374u, 366u, 2350u, 2350u, 2350u, 665u, 515u, 590u,
 2350u, 2350u, 2350u, 2350u, 2350u, 2350u, 2350u, 2350u, 2350u, 2350u, 2350u, 2350u, 2350u, 2350u, 452u, 2350u,
 2350u, 485u, 2350u, 2350u, 2350u, 2350u, 1070u, 2350u, 2350u, 2350u, 2350u, 1582u, 2350u, 2350u, 2350u, 2350u,
 2094u, 2350u, 2350u, 2350u, 2350u
};


//#line 1 "c:/etc/yylex.c"
/*
 * Copyright 1988, 1992 by Mortice Kern Systems Inc.  All rights reserved.
 * All rights reserved.
 *
 * $Header: /u/rd/src/lex/rcs/CMDlex.c 1.57 1995/12/11 22:14:06 fredw Exp $
 *
 */
#include <stdlib.h>
#include <stdio.h>
#if	__STDC__
#define YY_ARGS(args)	args
#else
#define YY_ARGS(args)	()
#endif

#ifdef LEX_WINDOWS
#include <windows.h>

/*
 * define, if not already defined
 * the flag YYEXIT, which will allow
 * graceful exits from CMDlex()
 * without resorting to calling exit();
 */

#ifndef YYEXIT
#define YYEXIT	1
#endif

/*
 * the following is the handle to the current
 * instance of a windows program. The user
 * program calling CMDlex must supply this!
 */

#ifdef STRICT
extern HINSTANCE hInst;	
#else
extern HANDLE hInst;	
#endif

#endif	/* LEX_WINDOWS */

/*
 * Define m_textmsg() to an appropriate function for internationalized messages
 * or custom processing.
 */
#ifndef I18N
#define	m_textmsg(id, str, cls)	(str)
#else /*I18N*/
extern	char* m_textmsg YY_ARGS((int id, const char* str, char* cls));
#endif/*I18N*/

/*
 * Include string.h to get definition of memmove() and size_t.
 * If you do not have string.h or it does not declare memmove
 * or size_t, you will have to declare them here.
 */
#include <string.h>
/* Uncomment next line if memmove() is not declared in string.h */
/*extern char * memmove();*/
/* Uncomment next line if size_t is not available in stdio.h or string.h */
/*typedef unsigned size_t;*/
/* Drop this when LATTICE provides memmove */
#ifdef LATTICE
#define memmove	memcopy
#endif

/*
 * YY_STATIC determines the scope of variables and functions
 * declared by the lex scanner. It must be set with a -DYY_STATIC
 * option to the compiler (it cannot be defined in the lex program).
 */
#ifdef	YY_STATIC
/* define all variables as static to allow more than one lex scanner */
#define	YY_DECL	static
#else
/* define all variables as global to allow other modules to access them */
#define	YY_DECL	
#endif

/*
 * You can redefine CMDgetc. For YACC Tracing, compile this code
 * with -DYYTRACE to get input from yt_getc
 */
#ifdef YYTRACE
extern int	yt_getc YY_ARGS((void));
#define CMDgetc()	yt_getc()
#else
#define	CMDgetc()	getc(CMDin) 	/* CMDlex input source */
#endif

/*
 * the following can be redefined by the user.
 */
#ifdef YYEXIT
#define	YY_FATAL(msg)	{ fprintf(CMDout, "CMDlex: %s\n", msg); CMDLexFatal = 1; }
#else /* YYEXIT */
#define	YY_FATAL(msg)	{ fprintf(stderr, "CMDlex: %s\n", msg); exit(1); }
#endif /* YYEXIT */

#undef ECHO
#define	ECHO		fputs(CMDtext, CMDout)

#define	output(c)	putc((c), CMDout) /* CMDlex sink for unmatched chars */
#define	YY_INTERACTIVE	1		/* save micro-seconds if 0 */

#define	BEGIN		CMD_start =
#define	REJECT		goto CMD_reject
#define	NLSTATE		(CMD_lastc = YYNEWLINE)
#define	YY_INIT \
	(CMD_start = CMDleng = CMD_end = 0, CMD_lastc = YYNEWLINE)
#define	CMDmore()	goto CMD_more
#define	CMDless(n)	if ((n) < 0 || (n) > CMD_end) ; \
			else { YY_SCANNER; CMDleng = (n); YY_USER; }

YY_DECL	void	CMD_reset YY_ARGS((void));
YY_DECL	int	input	YY_ARGS((void));
YY_DECL	int	unput	YY_ARGS((int c));

/* functions defined in libl.lib */
extern	int	CMDwrap	YY_ARGS((void));
extern	void	CMDerror	YY_ARGS((char *fmt, ...));
extern	void	CMDcomment	YY_ARGS((char *term));
extern	int	CMDmapch	YY_ARGS((int delim, int escape));

//#line 1 "code\scan.l"

#define YYLMAX 4096

#include <stdio.h>
#include "console.h"
#include "ast.h"
#include "gram.h"
#include "filstrm.h"
#include "talgorithm.h"
#include "resManager.h"
#include "consoleInternal.h"

static int Sc_ScanString();
static int Sc_ScanChar();
static int Sc_ScanNum();
static int Sc_ScanVar();
static int Sc_ScanHex();

#ifdef __BORLANDC__
#pragma option -w-rch
#endif

#define FLEX_DEBUG 1

//#undef input
//#undef unput
#undef CMDgetc
int CMDgetc();
static int lineIndex;

//#line 127 "c:/etc/yylex.c"


#ifndef YYLMAX
#define	YYLMAX		100		/* token and pushback buffer size */
#endif /* YYLMAX */

/*
 * If %array is used (or defaulted), CMDtext[] contains the token.
 * If %pointer is used, CMDtext is a pointer to CMD_tbuf[].
 */
YY_DECL char	CMDtext[YYLMAX+1];



#ifdef	YY_DEBUG
#undef	YY_DEBUG
#define	YY_DEBUG(fmt, a1, a2)	fprintf(stderr, fmt, a1, a2)
#else
#define	YY_DEBUG(fmt, a1, a2)
#endif

/*
 * The declaration for the lex scanner can be changed by
 * redefining YYLEX or YYDECL. This must be done if you have
 * more than one scanner in a program.
 */
#ifndef	YYLEX
#define	YYLEX CMDlex			/* name of lex scanner */
#endif

#ifndef YYDECL
#define	YYDECL	int YYLEX YY_ARGS((void))	/* declaration for lex scanner */
#endif

/*
 * stdin and stdout may not neccessarily be constants.
 * If stdin and stdout are constant, and you want to save a few cycles, then
 * #define YY_STATIC_STDIO 1 in this file or on the commandline when
 * compiling this file
 */
#ifndef YY_STATIC_STDIO
#define YY_STATIC_STDIO	0
#endif

#if YY_STATIC_STDIO
YY_DECL	FILE   *CMDin = stdin;
YY_DECL	FILE   *CMDout = stdout;
#else
YY_DECL	FILE   *CMDin = (FILE *)0;
YY_DECL	FILE   *CMDout = (FILE *)0;
#endif
YY_DECL	int	CMDlineno = 1;		/* line number */

/* CMD_sbuf[0:CMDleng-1] contains the states corresponding to CMDtext.
 * CMDtext[0:CMDleng-1] contains the current token.
 * CMDtext[CMDleng:CMD_end-1] contains pushed-back characters.
 * When the user action routine is active,
 * CMD_save contains CMDtext[CMDleng], which is set to '\0'.
 * Things are different when YY_PRESERVE is defined. 
 */
static	CMD_state_t CMD_sbuf [YYLMAX+1];	/* state buffer */
static	int	CMD_end = 0;		/* end of pushback */
static	int	CMD_start = 0;		/* start state */
static	int	CMD_lastc = YYNEWLINE;	/* previous char */
YY_DECL	int	CMDleng = 0;		/* CMDtext token length */
#ifdef YYEXIT
static	int CMDLexFatal;
#endif /* YYEXIT */

#ifndef YY_PRESERVE	/* the efficient default push-back scheme */

static	char CMD_save;	/* saved CMDtext[CMDleng] */

#define	YY_USER	{ /* set up CMDtext for user */ \
		CMD_save = CMDtext[CMDleng]; \
		CMDtext[CMDleng] = 0; \
	}
#define	YY_SCANNER { /* set up CMDtext for scanner */ \
		CMDtext[CMDleng] = CMD_save; \
	}

#else		/* not-so efficient push-back for CMDtext mungers */

static	char CMD_save [YYLMAX];
static	char *CMD_push = CMD_save+YYLMAX;

#define	YY_USER { \
		size_t n = CMD_end - CMDleng; \
		CMD_push = CMD_save+YYLMAX - n; \
		if (n > 0) \
			memmove(CMD_push, CMDtext+CMDleng, n); \
		CMDtext[CMDleng] = 0; \
	}
#define	YY_SCANNER { \
		size_t n = CMD_save+YYLMAX - CMD_push; \
		if (n > 0) \
			memmove(CMDtext+CMDleng, CMD_push, n); \
		CMD_end = CMDleng + n; \
	}

#endif


#ifdef LEX_WINDOWS

/*
 * When using the windows features of lex,
 * it is necessary to load in the resources being
 * used, and when done with them, the resources must
 * be freed up, otherwise we have a windows app that
 * is not following the rules. Thus, to make CMDlex()
 * behave in a windows environment, create a new
 * CMDlex() which will call the original CMDlex() as
 * another function call. Observe ...
 */

/*
 * The actual lex scanner (usually CMDlex(void)).
 * NOTE: you should invoke CMD_init() if you are calling CMDlex()
 * with new input; otherwise old lookaside will get in your way
 * and CMDlex() will die horribly.
 */
static int win_CMDlex();			/* prototype for windows CMDlex handler */

YYDECL {
	int wReturnValue;
	HANDLE hRes_table;
	unsigned short far *old_CMD_la_act;	/* remember previous pointer values */
	short far *old_CMD_final;
	CMD_state_t far *old_CMD_begin;
	CMD_state_t far *old_CMD_next;
	CMD_state_t far *old_CMD_check;
	CMD_state_t far *old_CMD_default;
	short far *old_CMD_base;

	/*
	 * the following code will load the required
	 * resources for a Windows based parser.
	 */

	hRes_table = LoadResource (hInst,
		FindResource (hInst, "UD_RES_CMDLEX", "CMDLEXTBL"));
	
	/*
	 * return an error code if any
	 * of the resources did not load
	 */

	if (hRes_table == NULL)
		return (0);
	
	/*
	 * the following code will lock the resources
	 * into fixed memory locations for the scanner
	 * (and remember previous pointer locations)
	 */

	old_CMD_la_act = CMD_la_act;
	old_CMD_final = CMD_final;
	old_CMD_begin = CMD_begin;
	old_CMD_next = CMD_next;
	old_CMD_check = CMD_check;
	old_CMD_default = CMD_default;
	old_CMD_base = CMD_base;

	CMD_la_act = (unsigned short far *)LockResource (hRes_table);
	CMD_final = (short far *)(CMD_la_act + Sizeof_CMD_la_act);
	CMD_begin = (CMD_state_t far *)(CMD_final + Sizeof_CMD_final);
	CMD_next = (CMD_state_t far *)(CMD_begin + Sizeof_CMD_begin);
	CMD_check = (CMD_state_t far *)(CMD_next + Sizeof_CMD_next);
	CMD_default = (CMD_state_t far *)(CMD_check + Sizeof_CMD_check);
	CMD_base = (CMD_state_t far *)(CMD_default + Sizeof_CMD_default);


	/*
	 * call the standard CMDlex() code
	 */

	wReturnValue = win_CMDlex();

	/*
	 * unlock the resources
	 */

	UnlockResource (hRes_table);

	/*
	 * and now free the resource
	 */

	FreeResource (hRes_table);

	/*
	 * restore previously saved pointers
	 */

	CMD_la_act = old_CMD_la_act;
	CMD_final = old_CMD_final;
	CMD_begin = old_CMD_begin;
	CMD_next = old_CMD_next;
	CMD_check = old_CMD_check;
	CMD_default = old_CMD_default;
	CMD_base = old_CMD_base;

	return (wReturnValue);
}	/* end function */

static int win_CMDlex() {

#else /* LEX_WINDOWS */

/*
 * The actual lex scanner (usually CMDlex(void)).
 * NOTE: you should invoke CMD_init() if you are calling CMDlex()
 * with new input; otherwise old lookaside will get in your way
 * and CMDlex() will die horribly.
 */
YYDECL {

#endif /* LEX_WINDOWS */

	register int c, i, CMDbase;
	unsigned	CMDst;	/* state */
	int CMDfmin, CMDfmax;	/* CMD_la_act indices of final states */
	int CMDoldi, CMDoleng;	/* base i, CMDleng before look-ahead */
	int CMDeof;		/* 1 if eof has already been read */
//#line 48 "code\scan.l"
	;

//#line 350 "c:/etc/yylex.c"



#if !YY_STATIC_STDIO
	if (CMDin == (FILE *)0)
		CMDin = stdin;
	if (CMDout == (FILE *)0)
		CMDout = stdout;
#endif

#ifdef YYEXIT
	CMDLexFatal = 0;
#endif /* YYEXIT */

	CMDeof = 0;
	i = CMDleng;
	YY_SCANNER;

  CMD_again:
	CMDleng = i;
	/* determine previous char. */
	if (i > 0)
		CMD_lastc = CMDtext[i-1];
	/* scan previously accepted token adjusting CMDlineno */
	while (i > 0)
		if (CMDtext[--i] == YYNEWLINE)
			CMDlineno++;
	/* adjust pushback */
	CMD_end -= CMDleng;
	if (CMD_end > 0)
		memmove(CMDtext, CMDtext+CMDleng, (size_t) CMD_end);
	i = 0;

  CMD_contin:
	CMDoldi = i;

	/* run the state machine until it jams */
	CMDst = CMD_begin[CMD_start + ((CMD_lastc == YYNEWLINE) ? 1 : 0)];
	CMD_sbuf[i] = (CMD_state_t) CMDst;
	do {
		YY_DEBUG(m_textmsg(1547, "<state %d, i = %d>\n", "I num1 num2"), CMDst, i);
		if (i >= YYLMAX) {
			YY_FATAL(m_textmsg(1548, "Token buffer overflow", "E"));
#ifdef YYEXIT
			if (CMDLexFatal)
				return -2;
#endif /* YYEXIT */
		}	/* endif */

		/* get input char */
		if (i < CMD_end)
			c = CMDtext[i];		/* get pushback char */
		else if (!CMDeof && (c = CMDgetc()) != EOF) {
			CMD_end = i+1;
			CMDtext[i] = (char) c;
		} else /* c == EOF */ {
			c = EOF;		/* just to make sure... */
			if (i == CMDoldi) {	/* no token */
				CMDeof = 0;
				if (CMDwrap())
					return 0;
				else
					goto CMD_again;
			} else {
				CMDeof = 1;	/* don't re-read EOF */
				break;
			}
		}
		YY_DEBUG(m_textmsg(1549, "<input %d = 0x%02x>\n", "I num hexnum"), c, c);

		/* look up next state */
		while ((CMDbase = CMD_base[CMDst]+(unsigned char)c) > CMD_nxtmax
		    || CMD_check[CMDbase] != (CMD_state_t) CMDst) {
			if (CMDst == CMD_endst)
				goto CMD_jammed;
			CMDst = CMD_default[CMDst];
		}
		CMDst = CMD_next[CMDbase];
	  CMD_jammed: ;
	  CMD_sbuf[++i] = (CMD_state_t) CMDst;
	} while (!(CMDst == CMD_endst || YY_INTERACTIVE && CMD_base[CMDst] > CMD_nxtmax && CMD_default[CMDst] == CMD_endst));
	YY_DEBUG(m_textmsg(1550, "<stopped %d, i = %d>\n", "I num1 num2"), CMDst, i);
	if (CMDst != CMD_endst)
		++i;

  CMD_search:
	/* search backward for a final state */
	while (--i > CMDoldi) {
		CMDst = CMD_sbuf[i];
		if ((CMDfmin = CMD_final[CMDst]) < (CMDfmax = CMD_final[CMDst+1]))
			goto CMD_found;	/* found final state(s) */
	}
	/* no match, default action */
	i = CMDoldi + 1;
	output(CMDtext[CMDoldi]);
	goto CMD_again;

  CMD_found:
	YY_DEBUG(m_textmsg(1551, "<final state %d, i = %d>\n", "I num1 num2"), CMDst, i);
	CMDoleng = i;		/* save length for REJECT */
	
	/* pushback look-ahead RHS */
	if ((c = (int)(CMD_la_act[CMDfmin]>>9) - 1) >= 0) { /* trailing context? */
		unsigned char *bv = CMD_look + c*YY_LA_SIZE;
		static unsigned char bits [8] = {
			1<<0, 1<<1, 1<<2, 1<<3, 1<<4, 1<<5, 1<<6, 1<<7
		};
		while (1) {
			if (--i < CMDoldi) {	/* no / */
				i = CMDoleng;
				break;
			}
			CMDst = CMD_sbuf[i];
			if (bv[(unsigned)CMDst/8] & bits[(unsigned)CMDst%8])
				break;
		}
	}

	/* perform action */
	CMDleng = i;
	YY_USER;
	switch (CMD_la_act[CMDfmin] & 0777) {
	case 0:
//#line 49 "code\scan.l"
	{ }
	break;
	case 1:
//#line 50 "code\scan.l"
	;
	break;
	case 2:
//#line 51 "code\scan.l"
	;
	break;
	case 3:
//#line 52 "code\scan.l"
	;
	break;
	case 4:
//#line 53 "code\scan.l"
	{lineIndex++;}
	break;
	case 5:
//#line 54 "code\scan.l"
	{ return(Sc_ScanString()); }
	break;
	case 6:
//#line 55 "code\scan.l"
	{ return(Sc_ScanChar()); }
	break;
	case 7:
//#line 56 "code\scan.l"
	return(CMDlval.i = opEQ);
	break;
	case 8:
//#line 57 "code\scan.l"
	return(CMDlval.i = opNE);
	break;
	case 9:
//#line 58 "code\scan.l"
	return(CMDlval.i = opGE);
	break;
	case 10:
//#line 59 "code\scan.l"
	return(CMDlval.i = opLE);
	break;
	case 11:
//#line 60 "code\scan.l"
	return(CMDlval.i = opAND);
	break;
	case 12:
//#line 61 "code\scan.l"
	return(CMDlval.i = opOR);
	break;
	case 13:
//#line 62 "code\scan.l"
	return(CMDlval.i = opCOLONCOLON);
	break;
	case 14:
//#line 63 "code\scan.l"
	return(CMDlval.i = opMINUSMINUS);
	break;
	case 15:
//#line 64 "code\scan.l"
	return(CMDlval.i = opPLUSPLUS);
	break;
	case 16:
//#line 65 "code\scan.l"
	return(CMDlval.i = opCAT);
	break;
	case 17:
//#line 66 "code\scan.l"
	return(CMDlval.i = opSHL);
	break;
	case 18:
//#line 67 "code\scan.l"
	return(CMDlval.i = opSHR);
	break;
	case 19:
//#line 68 "code\scan.l"
	return(CMDlval.i = opPLASN);
	break;
	case 20:
//#line 69 "code\scan.l"
	return(CMDlval.i = opMIASN);
	break;
	case 21:
//#line 70 "code\scan.l"
	return(CMDlval.i = opMLASN);
	break;
	case 22:
//#line 71 "code\scan.l"
	return(CMDlval.i = opDVASN);
	break;
	case 23:
//#line 72 "code\scan.l"
	return(CMDlval.i = opMODASN);
	break;
	case 24:
//#line 73 "code\scan.l"
	return(CMDlval.i = opANDASN);
	break;
	case 25:
//#line 74 "code\scan.l"
	return(CMDlval.i = opXORASN);
	break;
	case 26:
//#line 75 "code\scan.l"
	return(CMDlval.i = opORASN);
	break;
	case 27:
//#line 76 "code\scan.l"
	return(CMDlval.i = opSLASN);
	break;
	case 28:
//#line 77 "code\scan.l"
	return(CMDlval.i = opSRASN);
	break;
	case 29:
	case 30:
	case 31:
	case 32:
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
	case 38:
	case 39:
	case 40:
	case 41:
	case 42:
	case 43:
	case 44:
	case 45:
	case 46:
	case 47:
	case 48:
	case 49:
	case 50:
	case 51:
	case 52:
//#line 101 "code\scan.l"
	{       return(CMDlval.i = CMDtext[0]); }
	break;
	case 53:
//#line 103 "code\scan.l"
	{ CMDlval.i = lineIndex; return(rwBREAK); }
	break;
	case 54:
//#line 104 "code\scan.l"
	{ CMDlval.i = lineIndex; return(rwNIL); }
	break;
	case 55:
//#line 105 "code\scan.l"
	{ CMDlval.i = lineIndex; return(rwCLASS); }
	break;
	case 56:
//#line 106 "code\scan.l"
	{ CMDlval.i = lineIndex; return(rwRETURN); }
	break;
	case 57:
//#line 107 "code\scan.l"
	{ CMDlval.i = lineIndex; return(rwELSE); }
	break;
	case 58:
//#line 108 "code\scan.l"
	{ CMDlval.i = lineIndex; return(rwGLOBAL); }
	break;
	case 59:
//#line 109 "code\scan.l"
	{ CMDlval.i = lineIndex; return(rwWHILE); }
	break;
	case 60:
//#line 110 "code\scan.l"
	{ CMDlval.i = lineIndex; return(rwIF); }
	break;
	case 61:
//#line 111 "code\scan.l"
	{ CMDlval.i = lineIndex; return(rwFOR); }
	break;
	case 62:
//#line 112 "code\scan.l"
	{ CMDlval.i = lineIndex; return(rwCONTINUE); }
	break;
	case 63:
//#line 113 "code\scan.l"
	{ CMDlval.i = lineIndex; return(rwDEFINE); }
	break;
	case 64:
//#line 114 "code\scan.l"
	{ CMDlval.i = lineIndex; return(rwDECLARE); }
	break;
	case 65:
//#line 115 "code\scan.l"
	return(Sc_ScanVar());
	break;
	case 66:
//#line 116 "code\scan.l"
	{ CMDtext[CMDleng] = 0; CMDlval.s = stringTable.insert(CMDtext); return(TTAG); }
	break;
	case 67:
//#line 117 "code\scan.l"
	{ CMDtext[CMDleng] = 0; CMDlval.s = stringTable.insert(CMDtext); return(IDENT); }
	break;
	case 68:
//#line 118 "code\scan.l"
	return(Sc_ScanHex());
	break;
	case 69:
//#line 119 "code\scan.l"
	{ CMDtext[CMDleng] = 0; CMDlval.i = atoi(CMDtext); return INTCONST; }
	break;
	case 70:
//#line 120 "code\scan.l"
	return Sc_ScanNum();
	break;
	case 71:
//#line 121 "code\scan.l"
	return(ILLEGAL_TOKEN);
	break;
	case 72:
//#line 122 "code\scan.l"
	return(ILLEGAL_TOKEN);
	break;

//#line 472 "c:/etc/yylex.c"

	}
	YY_SCANNER;
	i = CMDleng;
	goto CMD_again;			/* action fell though */

	// Altimor: these were unused
  //CMD_reject:
	YY_SCANNER;
	i = CMDoleng;			/* restore original CMDtext */
	if (++CMDfmin < CMDfmax)
		goto CMD_found;		/* another final state, same length */
	else
		goto CMD_search;		/* try shorter CMDtext */

  //CMD_more:
	YY_SCANNER;
	i = CMDleng;
	if (i > 0)
		CMD_lastc = CMDtext[i-1];
	goto CMD_contin;
}
/*
 * Safely switch input stream underneath LEX
 */
typedef struct CMD_save_block_tag {
	FILE	* oldfp;
	int	oldline;
	int	oldend;
	int	oldstart;
	int	oldlastc;
	int	oldleng;
	char	savetext[YYLMAX+1];
	CMD_state_t	savestate[YYLMAX+1];
} YY_SAVED;

void
CMD_reset()
{
	YY_INIT;
	CMDlineno = 1;		/* line number */
}

#if 0
YY_SAVED *
CMDSaveScan(fp)
FILE * fp;
{
	YY_SAVED * p;

	if ((p = (YY_SAVED *) malloc(sizeof(*p))) == NULL)
		return p;

	p->oldfp = CMDin;
	p->oldline = CMDlineno;
	p->oldend = CMD_end;
	p->oldstart = CMD_start;
	p->oldlastc = CMD_lastc;
	p->oldleng = CMDleng;
	(void) memcpy(p->savetext, CMDtext, sizeof CMDtext);
	(void) memcpy((char *) p->savestate, (char *) CMD_sbuf,
		sizeof CMD_sbuf);

	CMDin = fp;
	CMDlineno = 1;
	YY_INIT;

	return p;
}
/*f
 * Restore previous LEX state
 */
void
CMDRestoreScan(p)
YY_SAVED * p;
{
	if (p == NULL)
		return;
	CMDin = p->oldfp;
	CMDlineno = p->oldline;
	CMD_end = p->oldend;
	CMD_start = p->oldstart;
	CMD_lastc = p->oldlastc;
	CMDleng = p->oldleng;

	(void) memcpy(CMDtext, p->savetext, sizeof CMDtext);
	(void) memcpy((char *) CMD_sbuf, (char *) p->savestate,
		sizeof CMD_sbuf);
	free(p);
}
/*
 * User-callable re-initialization of CMDlex()
 */
/* get input char with pushback */
YY_DECL int
input()
{
	int c;
#ifndef YY_PRESERVE
	if (CMD_end > CMDleng) {
		CMD_end--;
		memmove(CMDtext+CMDleng, CMDtext+CMDleng+1,
			(size_t) (CMD_end-CMDleng));
		c = CMD_save;
		YY_USER;
#else
	if (CMD_push < CMD_save+YYLMAX) {
		c = *CMD_push++;
#endif
	} else
		c = CMDgetc();
	CMD_lastc = c;
	if (c == YYNEWLINE)
		CMDlineno++;
	if (c == EOF) /* CMDgetc() can set c=EOF vsc4 wants c==EOF to return 0 */
		return 0;
	else
		return c;
}

/*f
 * pushback char
 */
YY_DECL int
unput(c)
	int c;
{
#ifndef YY_PRESERVE
	if (CMD_end >= YYLMAX) {
		YY_FATAL(m_textmsg(1552, "Push-back buffer overflow", "E"));
	} else {
		if (CMD_end > CMDleng) {
			CMDtext[CMDleng] = CMD_save;
			memmove(CMDtext+CMDleng+1, CMDtext+CMDleng,
				(size_t) (CMD_end-CMDleng));
			CMDtext[CMDleng] = 0;
		}
		CMD_end++;
		CMD_save = (char) c;
#else
	if (CMD_push <= CMD_save) {
		YY_FATAL(m_textmsg(1552, "Push-back buffer overflow", "E"));
	} else {
		*--CMD_push = c;
#endif
		if (c == YYNEWLINE)
			CMDlineno--;
	}	/* endif */
	return c;
}

#endif

//#line 124 "code\scan.l"

/*
 * Scan character constant.
 */

/*
 * Scan identifier.
 */

static const char *scanBuffer;
static const char *fileName;
static int scanIndex;
 
void CMDerror(char * s, ...)
{
   s;
   CMDConsole::getLocked()->printf("%s Line: %d - Syntax error.",
      fileName, lineIndex);
}

void SetScanBuffer(const char *sb, const char *fn)
{
   scanBuffer = sb;
   fileName = fn;
   scanIndex = 0;
   lineIndex = 1;
}

int CMDgetc()
{
   int ret = scanBuffer[scanIndex];
   if(ret)
      scanIndex++;
   else
      ret = -1;
   return ret;
}

int CMDwrap()
{
   return 1;
}

static int Sc_ScanVar()
{
   CMDtext[CMDleng] = 0;
	CMDlval.s = stringTable.insert(CMDtext);
	return(VAR);
}
/*
 * Scan string constant.
 */

static int charConv(int in)
{
   switch(in)
   {
      case 'n':
         return '\n';
      case 't':
         return '\t';
      default:
         return in;
   }
}

static int getHexDigit(char c)
{
   if(c >= '0' && c <= '9')
      return c - '0';
   if(c >= 'A' && c <= 'F')
      return c - 'A' + 10;
   if(c >= 'a' && c <= 'f')
      return c - 'a' + 10;
   return -1;
}

static int Sc_ScanString()
{
	CMDtext[CMDleng - 1] = 0;
   int len = CMDleng - 1;
   int i;
   for(i = 1; i < len;)
   {
      if(CMDtext[i] == '\\')
      {
         if(CMDtext[i+1] == 'x')
         {
            int dig1 = getHexDigit(CMDtext[i+2]);
            if(dig1 == -1)
               return -1;

            int dig2 = getHexDigit(CMDtext[i+3]);
            if(dig2 == -1)
               return -1;
            CMDtext[i] = dig1 * 16 + dig2;
            memmove(CMDtext + i + 1, CMDtext + i + 4, len - i - 3);
            len -= 3;
            i++;
         }
         else
         {
            CMDtext[i] = charConv(CMDtext[i+1]);
            memmove(CMDtext + i + 1, CMDtext + i + 2, len - i - 1);
            len--;
            i++;
         }
      }
      else
         i++;
   }


	CMDlval.str = new char[strlen(CMDtext)];
   strcpy(CMDlval.str, CMDtext + 1);
	return(STRATOM);
}

static int Sc_ScanChar()
{
   CMDtext[CMDleng-1] = 0;
	CMDlval.str = new char[strlen(CMDtext)];
   strcpy(CMDlval.str, CMDtext + 1);
	return(STRATOM);
}

static int Sc_ScanNum()
{
   CMDtext[CMDleng] = 0;
	CMDlval.f = atof(CMDtext);
	return(FLTCONST);
}

static int Sc_ScanHex()
{
   int val = 0;
   sscanf(CMDtext, "%x", &val);
   CMDlval.i = val;
   return INTCONST;
}

