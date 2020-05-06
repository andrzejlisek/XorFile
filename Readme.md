# Assumptions and purpose

XorFile is pusposed to protect data archives from loss caused by storage lose or damage\. Archive can consist of several data storages, where every storage is similar capacity, data can be divided into equally sized packets, which size must be smaller than smallest data storage\.

There is data packets and one or two XOR packets\. If real data size is smaller than desired packet, the remaining packet part are treatet as zero sequence without write possibility\. The number of data packet can be at the most 15\. If exists one XOR packet, there can be recovered one missing data packet, if exists two XOR packets, there can be recovered one or two missing data packets\. The recovery process uses all surviving data packets and XOR packets\.

All operations in XorFile are designed to do sequentially in steps\. Doing one step requires avaibility to only two packets simultaneously, so using this application is possible even, if there is possible to attach only two data storages and one data storage can be replaced between two steps\.

# Mangle function and derivatives

The mangle function operates on 4\-bit numbers\. Calculating one byte \(8\-bit number\) is done by two separated calculations for lower and upper 4\-bit parts\.

If `b3`, `b2`, `b1` and `b0` means four bits of number, the mangle function can be defined using the following formula:

```
M(b3, b2, b1, b0) = (b3 + b0, b3, b2, b1)
```

The same function can be defined explicity as value array:

```
M(0000b) = 0000b
M(0001b) = 1000b
M(0010b) = 0001b
M(0011b) = 1001b
M(0100b) = 0010b
M(0101b) = 1010b
M(0110b) = 0011b
M(0111b) = 1011b
M(1000b) = 1100b
M(1001b) = 0100b
M(1010b) = 1101b
M(1011b) = 0101b
M(1100b) = 1110b
M(1101b) = 0110b
M(1110b) = 1111b
M(1111b) = 0111b
```

This function has cycle consisting of all possible 15 4\-bit numbers other than 0\. Mangle of 0 gives 0, but mangle of number other than 0 gives the next number in this cycle\.

The cycle is following:

```
0001b -> 1000b -> 1100b -> 1110b -> 1111b -> 0111b -> 1011b -> 0101b -> 1010b -> 1101b -> 0110b -> 0011b -> 1001b -> 0100b -> 0010b -> 0001b
```

The mangle function has two\-argument version function defined by following formulas:

```
M(0, x) = x
M(1, x) = M(x)
M(2, x) = M(M(x))
M(3, x) = M(M(M(x)))
...
M(14, x) = M(M(M(M(M(M(M(M(M(M(M(M(M(M(x))))))))))))))
```

Because existence of this cycle, the following formulas are true:

```
M(15, x) = M(0, x)
M(16, x) = M(1, x)
M(n, x) = M(n - 15, x) where n >= 15
```

In recovery process using Q XOR packet, there is used unmangle function signed as `UM(n, x)`, which is inverse function to mangle\. Unmangle function meets formulas:

```
M(x) = y <=> UM(y) = x
UM(M(x)) = x
M(UM(x)) = x
```

The 2\-argument version of unmangle function meets following formulas:

```
M(n, x) = y <=> UM(n, y) = x
UM(n, M(n, x)) = x
M(n, UM(n, x)) = x
```

In 4\-bit mangle function, there is simple dependance between mangle and unmangle function:

```
UM(0, x) = M(15, x) = M(0, x) = x
UM(1, x) = M(14, x)
UM(2, x) = M(13, x)
UM(3, x) = M(12, x)
UM(n, x) = M(15 - n, x)
```

The unmangle function has the same cycle as mangle function, but with opposite direction, so the moth functions can be defined using the same cycle array\.

In recovering two data packet using both XOR packets, there is used XMangle function, which meets following formula, using xor bitwise operation designed as ^:

```
x = XM(n, x ^ M(n ,x))
```

The XMangle function can be defined as multiple Mangle function:

```
XM(n, x) = M(k, x)
```

For every `n`, which `n >= 0` and `n <= 14`, there exists ony one `k >= 0`, which `k >= 0` and `k <= 14`, and `k` value can be found by brute\-force method\.

# P and Q calculation principle

There are two XOR packets, named `P` and `Q`, and data packets named from `D0` to `D14`\. The number of data packets is limited by mangle function cycle length\.

The mangle function is used to calculate `Q` packet and recovery data packet using `Q` packet `Dn` is the last packet in specified bunch, `n` is from 0 to 14\. The `^` symbol in formulas denotes xor bitwise operation\.

The XOR packets are calculated using following formulas:

```
P = D0 ^ D1 ^ D2 ^ ... ^ Dn
Q = M(0, D0) ^ M(1, D1) ^ M(2, D2) ^ ... ^ M(n, Dn)
```

# P and Q calculation operation sequence

The `P` calculation sequence in bunch of n data packet consists of following steps:


1. Create P as filled with zeros
2. Rewrite P as `P ^ D0`
3. Rewrite P as `P ^ D1`
4. \.\.\.
5. Rewrite P as `P ^ Dn`

The `Q` calculation sequence in bunch of n data packet consists of following steps:


1. Create P as filled with zeros
2. Rewrite P as `P ^ M(0, D0)`
3. Rewrite P as `P ^ M(1, D1)`
4. \.\.\.
5. Rewrite P as `P ^ M(n, Dn)`

# XOR operation and mangle function features used in data packets recovery

The xor operation is commutative and associative, and xor operation with the same number gives 0, so the following equivalences are true:

```
t1 ^ t2 = 0 <=> t1 = t2
t1 ^ t2 ^ t3 = 0 <=> t1 = t2 ^ t3 <=> t1 ^ t2 = t3
t1 ^ t2 ^ .. tn ^ t(n+1) ^ t(n+2) ^ .. ^ t(n+k) = 0 <=> t1 ^ t2 ^ .. tn = t(n+1) ^ t(n+2) ^ .. ^ t(n+k)
```

The mangle function is distributive over xor as follows:

```
M(t1) ^ M(t2) ^ .. ^ M(tk) = M(t1 ^ t2 ^ .. ^ tk)
```

The mangle function is injective, so this equivalences are true:

```
t1 = t2 <=> M(t1) = M(t2)
t1 ^ t2 = t3 ^ t4 <=> M(t1) ^ M(t2) = M(t3) ^ M(t4)
```

# One data packet recovery principle using the P packet

In bunch of `n` data packet with `P` packet, where `k < n`, all packets meets formula:

```
D0 ^ D1 ^ ... ^ Dk ^ ... ^ Dn = P
```

If the `Dk` value is unknown, the above formula can be transformed using commutative and associative xor property:

```
D0 ^ D1 ^ ... ^ Dk ^ ... ^ Dn ^ P = 0
Dk = D0 ^ D1 ^ ... ^ Dn ^ P
```

# One data packet recovery operation sequence using the P packet

Recovering `Dk` packet from bunch of `n` packets consists of following steps:


1. Create `Dk` packet as the same as `P`
2. Rewrite `Dk` as result of `Dk ^ D0`
3. Rewrite `Dk` as result of `Dk ^ D1`
4. \.\.\.
5. Rewrite `Dk` as result of `Dk ^ Dn`

# One data packet recovery principle using the Q packet

In bunch of `n` data packet with `Q` packet, where `k < n`, all packets meets formula:

```
M(0, D0) ^ M(1, D1) ^ ... ^ M(k, Dk) ^ ... ^ M(n, Dn) = Q
```

If the `Dk` value is unknown, the above formula can be transformed using commutative and associative xor property:

```
M(0, D0) ^ M(1, D1) ^ ... ^ M(k, Dk) ^ ... ^ M(n, Dn) ^ Q = 0
M(k, Dk) = M(0, D0) ^ M(1, D1) ^ ... ^ M(n, Dn) ^ Q
```

When `M(k, Dk)` is known, the `Dk` can be calculated using unmangle function:

```
Dk = UM(k, M(k, Dk))
Dk = UM(k, M(0, D0) ^ M(1, D1) ^ ... ^ M(n, Dn) ^ Q)
```

# One data packet recovery operation sequence using the Q packet

Recovering `Dk` packet from bunch of `n` packets consists of following steps:


1. Create `Dk` packet as the same as `Q`
2. Rewrite `Dk` as result of `Dk ^ M(0, D0)`
3. Rewrite `Dk` as result of `Dk ^ M(1, D1)`
4. \.\.\.
5. Rewrite `Dk` as result of `Dk ^ M(n, Dn)`
6. Rewrite `Dk` as result `UM(k, Dk)`

# Two data packets recovery principle using both P and Q

If `P` and `Q` is known, `k < `l and `l < n`, the whole bunch meets following system of equations:

```
D0 ^ D1 ^ ... ^ Dk ^ ... ^ Dl ^ ... ^ Dn = P
M(0, D0) ^ M(1, D1) ^ ... ^ M(k, Dk) ^ ... ^ M(l, Dl) ^ ... ^ M(n, Dn) = Q
```

If `Dk` and `Dl` are unknown, the `Dk` can be calculated without `Dl` value and `Dl` packet can be calculated without `Dk` value\. Using Mangle injectivity, the system of equations can be transformed to following two systems of equations \(the bottom equation is the same in both systems\):

```
M(k, D0) ^ M(k, D1) ^ ... ^ M(k, Dk) ^ ... ^ M(k, Dl) ^ ... ^ M(k, Dn) = M(k, P)
M(0, D0) ^ M(1, D1) ^ ... ^ M(k, Dk) ^ ... ^ M(l, Dl) ^ ... ^ M(n, Dn) = Q

M(l, D0) ^ M(l, D1) ^ ... ^ M(l, Dk) ^ ... ^ M(l, Dl) ^ ... ^ M(l, Dn) = M(l, P)
M(0, D0) ^ M(1, D1) ^ ... ^ M(k, Dk) ^ ... ^ M(l, Dl) ^ ... ^ M(n, Dn) = Q
```

The systems of equations can be transformed to single equations by calculating xor of both sides:

```
M(k, D0) ^ M(k, D1) ^ ... ^ M(k, Dk) ^ ... ^ M(k, Dl) ^ ... ^ M(k, Dn) ^ M(0, D0) ^ M(1, D1) ^ ... ^ M(k, Dk) ^ ... ^ M(l, Dl) ^ ... ^ M(n, Dn) = M(k, P) ^ Q
M(l, D0) ^ M(l, D1) ^ ... ^ M(l, Dk) ^ ... ^ M(l, Dl) ^ ... ^ M(l, Dn) ^ M(0, D0) ^ M(1, D1) ^ ... ^ M(k, Dk) ^ ... ^ M(l, Dl) ^ ... ^ M(n, Dn) = M(l, P) ^ Q
```

Because the xor operation of two the same values gives 0, the values appearing twice can be reduced to get the following equations:

```
M(k, D0) ^ M(k, D1) ^ ... ^ M(k, Dl) ^ ... ^ M(k, Dn) ^ M(0, D0) ^ M(1, D1) ^ ... ^ M(l, Dl) ^ ... ^ M(n, Dn) = M(k, P) ^ Q
M(l, D0) ^ M(l, D1) ^ ... ^ M(l, Dk) ^ ... ^ M(l, Dn) ^ M(0, D0) ^ M(1, D1) ^ ... ^ M(k, Dk) ^ ... ^ M(n, Dn) = M(l, P) ^ Q
```

Each equation from above has only one unknown value per equation, `M(l, Dl)` and `M(k, Dk)`, so it can be transformed to get XOR of the values formula by transfer all unknown values to the left side and transfer all known values to the right side \(The `M(k, Dl)`, `M(l, Dl)`, `M(l, Dk)` and `M(k, Dk)` are excluded from the right side\):

```
M(k, Dl) ^ M(l, Dl) = M(k, D0) ^ M(k, D1) ^ ... ^ M(k, Dn) ^ M(0, D0) ^ M(1, D1) ^ ... ^ M(n, Dn) ^ M(k, P) ^ Q
M(k, Dk) ^ M(l, Dk) = M(l, D0) ^ M(l, D1) ^ ... ^ M(l, Dn) ^ M(0, D0) ^ M(1, D1) ^ ... ^ M(n, Dn) ^ M(l, P) ^ Q
```

Using unmangle function, the formulas with `Dk` and `Dl` can be prepared to use XMangle function by matching to `X ^ M(y, X)` pattern, where `X = Dk` or `X = Dl` and `y = l - k`:

```
U(k, M(k, Dl) ^ M(l, Dl)) = Dl ^ M(l - k, Dl)
U(k, M(k, Dk) ^ M(l, Dk)) = Dk ^ M(l - k, Dk)
```

The `Dk` and `Dl` can be calculated using XMangle function matching to the following equations:

```
Dl = XM(l - k, Dl ^ M(l - k, Dl))
Dk = XM(l - k, Dk ^ M(l - k, Dk))
```

Based on the above equations, the `Dl ^ M(l - k, Dl)` and the `Dk ^ M(l - k, Dk)` values can be replaced by formulas, which contains the `M(k, Dl) ^ M(l, Dl)` and `M(k, Dk) ^ M(l, Dk)` values, which are known:

```
Dl = XM(l - k, U(k, M(k, Dl) ^ M(l, Dl)))
Dk = XM(l - k, U(k, M(k, Dk) ^ M(l, Dk)))
```

# Two data packets recovery operation sequence using both P and Q

Recovering two data packets can be done in two phases:


1. The first phase recovers `Dk` packet by following steps:
   1. Create `Dk` as `M(l, P)`
   2. Rewrite `Dk` as `Dk ^ Q`
   3. Rewrite `Dk` as `Dk ^ M(l, D0) ^ M(0, D0)`
   4. Rewrite `Dk` as `Dk ^ M(l, D1) ^ M(1, D1)`
   5. \.\.\. \(ommit `Dk` and `Dl`\)
   6. Rewrite `Dk` as `Dk ^ M(l, Dn) ^ M(n, Dn)`
   7. Rewrite `Dk` as `XMangle(l - k, UM(k, Dk))`
2. The second phase recovers `Dl` packet by following steps:
   1. Create `Dl` as `M(k, P)`
   2. Rewrite `Dl` as `Dl ^ Q`
   3. Rewrite `Dl` as `Dl ^ M(k, D0) ^ M(0, D0)`
   4. Rewrite `Dl` as `Dl ^ M(k, D1) ^ M(1, D1)`
   5. \.\.\. \(ommit `Dk` and `Dl`\)
   6. Rewrite `Dl` as `Dl ^ M(k, Dn) ^ M(n, Dn)`
   7. Rewrite `Dl` as `XMangle(l - k, UM(k, Dl))`

# XorFile overwiew

XorFile interface consists of four tabs:


* **Define bunch** \- Purposed to define whole bunch of data packets\. One bunch can contain from 1 to 15 data packets with one or two parity packets\.
* **Recover missing or add/remove** \- Purposed to perform and monitor of all operations od data packets like add to bunch, remove from bunch, creating parity or recovering lost data packets\.
* **Test integrity** \- Purposed to test bunch, if P or Q packet is correct against current data packet\. This test can be done on packet fragments instead of whole packets\.
* **Index editor** \- Purposed to define, which files is used to create packet and test file integrity using digest computation\.

# Packet bunch

This **Define bunch** tab is used to define packet, which are used to make whole bunch\. Bunch definition can be saved to file, which can contain also data operation state\. The buttons performs following operations connected with files:


* **New** \- Creates new file \(clears bunch\)\.
* **Open** \- Opens existing file\.
* **Save** \- Saves current bunch to current file, if current file is not determined, this button will act as Save as\.
* **Save as** \- Saves current bunch to specified file\.

The **Data packets** field is used to set the number od data packets in current bunch\. and the bunch is presented in table below\. For every packet, you can define packet index file and base directory\.

The base directory is used to read and write packet files during any operation, using relative path defined in packet index file\.

The **Get sizes** button tries to read packet indices to get whole packet sizes\. The Load to index editor button loads selected packet to the index editor\.

For P or Q packet, index file may not be specified, so such packet is treated as not contained in bunch\.

The **Status** field is used to set packet status, which is used to generate operation scenario\. Status for defined packets can be one of following:


* **Exists** \- Packet files exists and can be used to any operation\.
* **Missing** \- Packet files are not exists and must be recovered\.
* **Add/Remove** \- Using to add packet to bunch or remove packet from bunch, both operations performs exactly the same operation on P and Q packets\.

The **Index file** field is used to select packet index file, which contains file names, sizes, position and digests\.

The **Base directory** field is used to set the base directory for file paths for index file\.

# P/Q create, data packet recovery or adding/removing data packets

Needed operation is defined using status definition for every packet on **Define bunch** tab\. On the **Recover missing or add/remove** tab, is is possible to prepare and perform this operation\.

To prepare operation scenario, you have to click **Generate scenario** button\. The table below will be cleared and filled in against operation scenario if operation is possible\. When operation is not possible \(for example, three data packets are missing\), appropriate message will be shown\.

Operation can be done for whole packet or for any packet fragment\. To set whole packet \(needed in most cases\), you have to click **Whole packet** button\. The **Begin offset** value will be 0 and the **End offset** value will be equal to the whole packet size decreased by 1\.

Every elementary operation has status, which can be changed using **Set as waiting**, **Set as done**, and **Set as postponed** buttons\.

Status are following:


* **Waiting** \- Operation is waiting to do an will be performed after clicking \_\_Perform waiting\_\_, after performing the status will be changed to Done\.
* **Done** \- Operation is done and should not be performed\.
* **Posponed** \- Operation will be ommited, but it should be performed later\.

The **Chunk size** set file chunk size read or written once, this may affect operation performance depending on working memory size and disk performance\.

For every operation, the progress bar will be displayed below the operation list\.

# Performing scenario with breaks and swiching the computer off between steps

In some cases, not all data packets are available simultaneously and changing disk requires computer to be switched off\. In this case, after scenario generating, you have to set **Waiting** status for this operations only, which can be performed without break and without any activities from user\.

The other operations has to be changed status to **Postponed**\. They, you can click **Perform waiting**\. After all waiting operations be performed \(status changed to **Done**\), you can save bunch definition to file to preserve current operation statuses\. Then you can switch the computer off, change disks and swith the computer on\.

After that, you have to run XorFile application and open last used bunch definition\. Then, you have to change all posponed operations, which can be performed on current disk configuration, to **waiting** status and click **Perform waiting** button\.

# Testing bunch integrity

XorFile allows check bunch integrity, i\.e\. check, if parity packets are correct against current data packets\. You can check whole packets \(one stripe from first to last byte\) or specified fragments \(stripes from specified to specified byte\)\. This procedure requires as free storage space as sum of all stripes sizes excepting of **P and Q sim**, which requires twice free storage space as sum of all stripes\.

This function can help in some cases, like, you not remember, if you added or removed data packet from bunch or it is possible, that at least one packet is unintentionally modified\.

This functionality is available on **Bunch integrity** tab\. In the upper table, you can view defined stripes\. To define stripes, you can use following buttons:


* **Add stripes** \- Add one or more stripes, enter parameters in following input boxex\.
* **Remove selected stripe** \- Removes selected stripe\.
* **Remove all stripes** \- Remove all stripes\.

To check consistency, you have to select one parity mode from following:


* **P only** \- Tests integrity against P packet only \- required the same space as one packet\.
* **Q only** \- Tests integrity against Q packet only \- required the same space as one packet\.
* **P and Q seq** \- Tests integrity against both parity packets sequentially \- required the same space as one packet and every data packet will be read twice\.
* **P and Q sim** \- Tests integrity against both parity packets simultaneously \- required the twice space as one packet and every data packet will be read once\.

After selecting mode, you have to click **Generate** to prepare test scenario\. To perform operation, you can use **Set as waiting**, **Set as done**, **Set as postponed** and **Perform waiting** buttons in the same way, as in P/Q create and data packet recovery scenarios\.

# Index editor

On the **Index editor** tab, you can edit packet contents and check integrity of files included in the packet\. One packet consists of files or blank spaces \(dummy files\)\. Every packet in the bunch must have the same size\. You can edit the packet index using following buttons:


* **Add existing file** \- Adds selected file, the name and sile will be read\.
* **Add new file** \- Adds new file, where name and size is given be user\. It is possible to add dummy file by giving blank file name and non\-zero size\.
* **Remove** \- Removes selected item\.
* **Move up** and **Move down** \- Moves selected item across the list\.
* **Get digest \- sel** \- Calculates the file digest for selected item and stores it in the index\.
* **Get digest \- all** \- Calculates the file digest for all items and stores it in the index\.
* **Test sizes** \- Checks, if the file exists and the phisical file sise is the same as file size defined in the index\.
* **Test digest \- sel** \- Calculates the digest for selected item and compares with the stored digest, the result will be displayed in the **Status** field\.
* **Test digest \- all** \- Calculates the digest for all items and compares with the stored digest, the result will be displayed in the **Status** field\.
* **Change digest** \- Allows to manually change digest stored in the index\.
* **Change size** \- Allows to change size of item, this operation does not affect the phisical file\.
* **Clear status** \- Clears the status field for all item, which is recommended after some activity filling in the field and before the next activity to clarity status information\.
* **To begin** \- Copys the value from **Begin** field to the **Begin offset** field on the **Recover missing or add/remove** tab\.
* **To end** \- Copys the value from **End** field to the **End offset** field on the **Recover missing or add/remove** tab\.

Below the mentioned buttons there is progress bar used during every operation\. Below this there are fields, which allows converting between absolute and relative paths of file names in the index:


* **Base directory** \- The path, for which will be related the relative path for file names\. You can select this directory using **Browse directory** button\.
* **Abs \-> Rel** \- Converts the absolute path to the relative path in the selected item\.
* **Rel \-> Abs** \- Converts the relative path to the absolute path in the selected item\.


