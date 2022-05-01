package ni.common.util;

import java.security.InvalidParameterException;
import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import java.security.*;

public class Hash extends ni.types.util.Hash {
    protected Hash() {
    }

    /**
     * A simple hash function from Robert Sedgwicks Algorithms in C book.
     */
    public static int hashRS(final byte[] str, final int len) {
        final int b = 378551;
        int a = 63689;
        int hash = 0;
        int i = 0;

        for (i = 0; i < len; i++) {
            hash = hash * a + str[i];
            a = a * b;
        }

        return (hash & 0x7FFFFFFF);
    }

    /**
     * A bitwise hash function written by Justin Sobel.
     */
    public static int hashJS(final byte[] str, final int len) {
        int hash = 1315423911;
        int i = 0;

        for (i = 0; i < len; i++) {
            hash ^= ((hash << 5) + str[i] + (hash >> 2));
        }

        return (hash & 0x7FFFFFFF);
    }

    /**
     * This hash algorithm is based on work by Peter J. Weinberger of AT&T Bell
     * Labs. The book Compilers (Principles, Techniques and Tools) by Aho, Sethi
     * and Ulman, recommends the use of hash functions thatemploy the hashing
     * methodology found in this particular algorithm.
     */
    public static int hashPJW(final byte[] str, final int len) {
        final int BitsInUnsignedInt = 32;
        final int ThreeQuarters = ((BitsInUnsignedInt * 3) / 4);
        final int OneEighth = (BitsInUnsignedInt / 8);
        final int HighBits = (0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
        int hash = 0;
        int test = 0;
        int i = 0;

        for (i = 0; i < len; i++) {
            hash = (hash << OneEighth) + str[i];

            if ((test = hash & HighBits) != 0) {
                hash = ((hash ^ (test >> ThreeQuarters)) & (~HighBits));
            }
        }

        return (hash & 0x7FFFFFFF);
    }

    /**
     * Similar to the PJW Hash function, but tweaked for 32-bit processors. Its
     * the hash function widely used on most UNIX systems.
     */
    public static int hashELF(final byte[] str, final int len) {
        int hash = 0;
        int x = 0;
        int i = 0;

        for (i = 0; i < len; i++) {
            hash = (hash << 4) + str[i];
            if ((x = (hash & 0xF0000000)) != 0) {
                hash ^= (x >> 24);
                hash &= ~x;
            }
        }

        return (hash & 0x7FFFFFFF);
    }

    /**
     * This hash function comes from Brian Kernighan and Dennis Ritchie's book
     * "The C Programming Language". It is a simple hash function using a
     * strange set of possible seeds which all constitute a pattern of
     * 31....31...31 etc, it seems to be very similar to the DJB hash function.
     */
    public static int hashBKDR(final byte[] str, final int len) {
        final int seed = 131; // 31 131 1313 13131 131313 etc..
        int hash = 0;
        int i = 0;
        for (i = 0; i < len; i++) {
            hash = (hash * seed) + str[i];
        }
        return (hash & 0x7FFFFFFF);
    }

    /**
     * This is the algorithm of choice which is used in the open source SDBM
     * project. The hash function seems to have a good over-all distribution for
     * many different data sets. It seems to work well in situations where there
     * is a high variance in the MSBs of the elements in a data set.
     */
    public static int hashSDBM(final byte[] str, final int len) {
        int hash = 0;
        int i = 0;
        for (i = 0; i < len; i++) {
            hash = str[i] + (hash << 6) + (hash << 16) - hash;
        }
        return (hash & 0x7FFFFFFF);
    }

    /**
     * An algorithm produced by Professor Daniel J. Bernstein and shown first to
     * the world on the usenet newsgroup comp.lang.c. It is one of the most
     * efficient hash functions ever published.
     */
    public static int hashDJB(final byte[] str, final int len) {
        int hash = 5381;
        int i = 0;
        for (i = 0; i < len; i++) {
            hash = ((hash << 5) + hash) + str[i];
        }
        return (hash & 0x7FFFFFFF);
    }

    /**
     * An algorithm proposed by Donald E. Knuth in The Art Of Computer
     * Programming Volume 3, under the topic of sorting and search chapter 6.4.
     */
    public static int hashDEK(final byte[] str, final int len) {
        int hash = len;
        int i = 0;
        for (i = 0; i < len; i++) {
            hash = ((hash << 5) ^ (hash >> 27)) ^ str[i];
        }
        return (hash & 0x7FFFFFFF);
    }

    /**
     * An algorithm produced by Arash Partow. He took ideas from all of the
     * above hash functions making a hybrid rotative and additive hash function
     * algorithm based around four primes 3,5,7 and 11. There isn't any real
     * mathematical analysis explaining why one should use this hash function
     * instead of the others described above other than the fact that I tired to
     * resemble the design as close as possible to a simple LFSR. An empirical
     * result which demonstrated the distributive abilities of the hash
     * algorithm was obtained using a hash-table with 100003 buckets, hashing
     * The Project Gutenberg Etext of Webster's Unabridged Dictionary, the
     * longest encountered chain length was 7, the average chain length was 2,
     * the number of empty buckets was 4579.
     */
    public static int hashAP(final byte[] str, final int len) {
        int hash = 0;
        int i = 0;

        for (i = 0; i < len; i++) {
            hash ^= ((i & 1) == 0) ? ((hash << 7) ^ str[i] ^ (hash >> 3)) :
                    (~((hash << 11) ^ str[i] ^ (hash >> 5)));
        }

        return (hash & 0x7FFFFFFF);
    }

    /**
     * A simple hash function to hash float arrays containing mathematical
     * vector/matrix.
     */
    public static int hashMath(final float[] v) {
        switch (v.length) {
        case 1:
            return hashVec(v[0]);
        case 2:
            return hashVec(v[0], v[1]);
        case 3:
            return hashVec(v[0], v[1], v[2]);
        case 4:
            return hashVec(v[0], v[1], v[2], v[3]);
        case 9:
            return hashMatrix(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8]);
        case 16:
            return hashMatrix(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
        default:
            throw new InvalidParameterException();
        }
    }

    public static String MD5(final String value) {
        return MD5(value.getBytes());
    }

    public static String MD5(final byte[] value) {
        try {
            final MessageDigest md = MessageDigest.getInstance("MD5");
            md.reset();
            return Strings.bytesToString(md.digest(value));
        }
        catch (final NoSuchAlgorithmException e) {
            Log.w("[NoSuchAlgorithmException]" + e.getMessage());
            return null;
        }
    }

    public static byte[] hash(final byte[] value, final String type) {
        try {
            final MessageDigest md = MessageDigest.getInstance(type);
            md.reset();
            return md.digest(value);
        }
        catch (final NoSuchAlgorithmException e) {
            Log.w("[NoSuchAlgorithmException]" + e.getMessage());
            return null;
        }
    }

    public static byte[] encrypt(final byte[] value, final String type, final byte[] key)
    {
        try {
            Mac mac = null;
            try {
                mac = Mac.getInstance(type);
            }
            catch (final NoSuchAlgorithmException e) {
                Log.w("[NoSuchAlgorithmException] " + e.getMessage());
                return null;
            }
            final SecretKeySpec secret = new SecretKeySpec(key, type);
            mac.init(secret);
            return mac.doFinal(value);
        }
        catch (final InvalidKeyException e) {
            Log.w("[InvalidKeyException]" + e.getMessage());
            return null;
        }
    }
}
