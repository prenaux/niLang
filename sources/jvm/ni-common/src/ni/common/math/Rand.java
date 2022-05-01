package ni.common.math;

import static ni.common.math.Vec.sigmoid;

import java.util.Collection;
import java.util.List;
import java.util.SortedSet;
import java.util.TreeSet;

public final class Rand extends java.util.Random {
    private static final long serialVersionUID = -4760294825656191581L;

    private static Rand singleton = null;

    public static Rand getSingleton() {
        if (singleton == null) {
            singleton = new Rand();
        }
        return singleton;
    }

    private volatile long state;

    public Rand() {
        state = xorShift64(System.currentTimeMillis() * System.currentTimeMillis());
    }

    public Rand(final long seed) {
        state = seed;
    }

    /**
     * Gets a long random value
     * 
     * @return Random long value based on static state
     */
    @Override
    public final long nextLong() {
        final long a = state;
        state = xorShift64(a);
        return a;
    }

    /**
     * XORShift algorithm - very fast psuedo-random number generator Credit to
     * George Marsaglia!
     * 
     * @param a Initial state
     * @return new state
     */
    public final long xorShift64(long a) {
        a ^= (a << 21);
        a ^= (a >>> 35);
        a ^= (a << 4);
        return a;
    }

    /**
     * XORShift algorithm - credit to George Marsaglia!
     * 
     * @param a Initial state
     * @return new state
     */
    public final int xorShift32(int a) {
        a ^= (a << 13);
        a ^= (a >>> 17);
        a ^= (a << 5);
        return a;
    }

    @Override
    protected int next(final int bits) {
        return (int)(nextLong() >>> (64 - bits));
    }

    @Override
    public void setSeed(final long seed) {
        state = seed;
    }

    public long getSeed() {
        return state;
    }

    @Override
    public boolean equals(final Object o) {
        if (o instanceof Rand)
            return equals(o);
        return super.equals(o);
    }

    public boolean equals(final Rand o) {
        return state == o.state;
    }

    @Override
    public int hashCode() {
        return ((int)state) ^ ((int)(state >>> 32));
    }

    /**
     * Returns true with a given probability
     * 
     * @param d
     * @return
     */
    public boolean chance(final double d) {
        return nextDouble() < d;
    }

    /**
     * Returns true with a given probability
     * 
     * @param d
     * @return
     */
    public boolean chance(final float d) {
        return nextFloat() < d;
    }

    /**
     * sum of best r from n s-sided dice
     * 
     * @param r
     * @param n
     * @param s
     * @return
     */
    public int best(final int r, final int n, final int s) {
        if ((n <= 0) || (r < 0) || (r > n) || (s < 0))
            return 0;

        final int[] rolls = new int[n];
        for (int i = 0; i < n; i++) {
            rolls[i] = d(s);
        }

        boolean found;
        do {
            found = false;
            for (int x = 0; x < n - 1; x++) {
                if (rolls[x] < rolls[x + 1]) {
                    final int t = rolls[x];
                    rolls[x] = rolls[x + 1];
                    rolls[x + 1] = t;
                    found = true;
                }
            }
        } while (found);

        int sum = 0;
        for (int i = 0; i < r; i++) {
            sum += rolls[i];
        }
        return sum;
    }

    /**
     * Poisson distribution
     * 
     * @param x
     * @return
     */
    public int po(final double x) {
        if (x <= 0) {
            if (x < 0)
                throw new IllegalArgumentException();
            return 0;
        }
        if (x > 400)
            return poLarge(x);
        return poMedium(x);
    }

    /**
     * logistic sigmoid probability
     * 
     * @param x
     * @return
     */
    public int sig(final float x) {
        return chance(sigmoid(x)) ? 1 : 0;
    }

    private int poMedium(final double x) {
        int r = 0;
        double a = nextDouble();
        double p = Math.exp(-x);

        while (a > p) {
            r++;
            a = a - p;
            p = p * x / r;
        }
        return r;
    }

    private int poLarge(final double x) {
        // normal approximation to poisson
        // strictly needed for x>=746 (=> Math.exp(-x)==0)
        return (int)(0.5 + n(x, Math.sqrt(x)));
    }

    public int po(final int numerator, final int denominator) {
        return po(((double)numerator) / denominator);
    }

    /**
     * calculates the sum of (number) s-sided dice
     * 
     * @param number
     * @param sides
     * @return
     */
    public int d(final int number, final int sides) {
        int total = 0;
        for (int i = 0; i < number; i++) {
            total += d(sides);
        }
        return total;
    }

    public static double factorial(final int n) {
        if (n < 2)
            return 1;
        double result = 1;
        for (int i = 2; i <= n; i++) {
            result *= i;
        }
        return result;
    }

    /**
     * Returns the probability of up to r sucesses in n trials
     * 
     * @param r
     * @param n
     * @param p
     * @return
     */
    public static double cumulativeBinomialChance(final int r, final int n, final double p)
    {
        double prob = 0;
        for (int i = 0; i <= r; i++) {
            prob += binomialChance(i, n, p);
        }
        return prob;
    }

    public static double combinations(final int r, final int n) {
        return factorial(n) / (factorial(r) * factorial(n - r));
    }

    public static double binomialChance(final int r, final int n, final double p) {
        return combinations(r, n) * Math.pow(p, r) * Math.pow((1 - p), n - r);
    }

    /**
     * Returns a random sample from an exponential distribution
     * 
     * @param mean
     * @return
     */
    public double exp(final double mean) {
        return -Math.log(nextDouble()) * mean;
    }

    /*
     *  Geometric distribution
     *  Discrete distribution with decay rate p
     *  Mean = (1-p) / p
     *  Probability mass function for each integer output k = p.(1-p)^k
     */
    public int geom(final double p) {
        return (int)Math.floor(Math.log(nextDouble()) / Math.log(1 - p));
    }

    /*
     * Generators for standard Java types
     */

    @Override
    public final int nextInt() {
        return (int)(nextLong() >> 32);
    }

    public final short nextShort() {
        return (short)(nextLong() >> 32);
    }

    public final char nextChar() {
        return (char)(nextLong() >> 32);
    }

    public final String nextLetterString(final int length) {
        final char[] cs = new char[length];
        for (int i = 0; i < length; i++) {
            cs[i] = nextLetter();
        }
        return new String(cs);
    }

    public final byte nextByte() {
        return (byte)(nextLong() >> 32);
    }

    @Override
    public boolean nextBoolean() {
        return (nextLong() & 65536) != 0;
    }

    public final char nextLetter() {
        return (char)range('a', 'z');
    }

    /**
     * Random number from zero to s-1
     * 
     * @param s Upper bound (excluded)
     * @return
     */
    public final int r(final int s) {
        if (s < 0)
            throw new IllegalArgumentException();
        final long result = ((nextLong() >>> 32) * s) >> 32;
        return (int)result;
    }

    /**
     * Return a random index between 0 and max (exclusive) not equal to i
     */
    public final int otherIndex(final int i, final int max) {
        return (r(max - 1) + i) % max;
    }

    private static final double DOUBLE_SCALE_FACTOR = 1.0 / Math.pow(2, 63);
    private static final float FLOAT_SCALE_FACTOR = (float)(1.0 / Math.pow(2, 63));

    /**
     * Returns standard double in range 0..1
     * 
     * @return
     */
    @Override
    public final double nextDouble() {
        return (nextLong() >>> 1) * DOUBLE_SCALE_FACTOR;
    }

    @Override
    public final float nextFloat() {
        return (nextLong() >>> 1) * FLOAT_SCALE_FACTOR;
    }

    public final double u() {
        return nextDouble();
    }

    public final double u(final double a) {
        return u(0, a);
    }

    public final double u(final double min, final double max) {
        return min + nextDouble() * (max - min);
    }

    /**
     * Randomly rounds to the nearest integer
     */
    public final int round(final double d) {
        final int i = (int)Math.floor(d);
        final int rem = (nextDouble() < (d - i)) ? 1 : 0;
        return i + rem;
    }

    /**
     * Returns random number uniformly distributed in inclusive [n1, n2] range.
     * It is allowed to have to n1 > n2, or n1 < n2, or n1 == n2.
     */
    public final int range(int n1, int n2) {
        if (n1 > n2) {
            final int t = n1;
            n1 = n2;
            n2 = t;
        }
        return n1 + r(n2 - n1 + 1);
    }

    /**
     * simulates a dice roll with the given number of sides
     * 
     * @param sides
     * @return
     */
    public final int d(final int sides) {
        return r(sides) + 1;
    }

    public final int d3() {
        return d(3);
    }

    public final int d4() {
        return d(4);
    }

    public final int d6() {
        return d(6);
    }

    public final int d8() {
        return d(8);
    }

    public final int d10() {
        return d(10);
    }

    public final int d12() {
        return d(12);
    }

    public final int d20() {
        return d(20);
    }

    public final int d100() {
        return d(100);
    }

    /**
     * Generates a normal distributed number with the given mean and standard
     * deviation
     * 
     * @param u Mean
     * @param sd Standard deviation
     * @return
     */
    public double n(final double u, final double sd) {
        return nextGaussian() * sd + u;
    }

    @Override
    public double nextGaussian() {
        // create a guassian random variable based on
        // Box-Muller transform
        double x, y, d2;
        do {
            // sample a point in the unit disc
            x = 2 * nextDouble() - 1;
            y = 2 * nextDouble() - 1;
            d2 = x * x + y * y;
        } while ((d2 > 1) || (d2 == 0));
        // create the radius factor
        final double radiusFactor = Math.sqrt(-2 * Math.log(d2) / d2);
        return x * radiusFactor;
        // could save and use the other value?
        // double anotherGaussian = y * radiusFactor;
    }

    public String nextString() {
        final char[] cs = new char[po(4)];
        for (int i = 0; i < cs.length; i++) {
            cs[i] = nextLetter();
        }
        return String.valueOf(cs);
    }

    /**
     * Shuffles all elements in a given array
     */
    public <T> void shuffle(final T[] ts) {
        for (int i = 0; i < (ts.length - 1); i++) {
            final int j = r(ts.length - i);
            if (i != j) {
                final T t = ts[i];
                ts[i] = ts[j];
                ts[j] = t;
            }
        }
    }

    /**
     * Chooses a set of distinct integers from a range 0 to maxValue-1 Resulting
     * integers are sorted
     */
    public void chooseIntegers(final int[] dest, final int destOffset, final int length, final int maxValue)
    {
        if (length > maxValue)
            throw new Error("Cannot choose " + length + " items from a set of " + maxValue);

        if (maxValue > (4 * length)) {
            chooseIntegersBySampling(dest, destOffset, length, maxValue);
            return;
        }

        chooseIntegersByExclusion(dest, destOffset, length, maxValue);

    }

    /**
     * Creates a set of n random integers from a range of 0 to maxValue-1
     */
    public void randIntegers(final int[] dest, final int destOffset, final int length, final int maxValue)
    {
        for (int i = destOffset; i < (destOffset + length); i++) {
            dest[i] = r(maxValue);
        }
    }

    public void chooseIntegersByExclusion(final int[] dest, final int destOffset, int n, int maxValue)
    {
        while (n > 0) {
            if ((n == maxValue) || (r(maxValue) < n)) {
                dest[destOffset + n - 1] = maxValue - 1;
                n--;
            }
            maxValue--;
        }
    }

    public void chooseIntegersByReservoirSampling(final int[] dest, final int destOffset, final int n, final int maxValue)
    {
        int found = 0;
        for (int i = 0; i < maxValue; i++) {
            if (found < n) {
                // fill up array
                dest[destOffset + found] = i;
                found++;
            }
            else {
                // replace with appropriate probability to ensure fair distribution
                final int ni = r(i + 1);
                if (ni < n) {
                    dest[destOffset + ni] = i;
                }
            }
        }
    }

    public void chooseIntegersBySampling(final int[] dest, int destOffset, final int n, final int maxValue)
    {
        final SortedSet<Integer> s = new TreeSet<Integer>();

        while (s.size() < n) {
            final int v = r(maxValue);
            s.add(v);
        }

        for (final Integer i : s) {
            dest[destOffset++] = i;
        }
    }

    /**
     * Picks a random item from a given array
     */
    public <T> T pick(final T[] ts) {
        return ts[r(ts.length)];
    }

    /**
     * Picks a random item from a given list
     */
    public <T> T pick(final List<T> ts) {
        return ts.get(r(ts.size()));
    }

    /**
     * Picks a random item from a given collection
     */
    public <T> T pick(final Collection<T> ts) {
        final int n = ts.size();
        if (n == 0)
            throw new Error("Empty collection!");
        int p = r(n);
        for (final T t : ts) {
            if (p-- == 0)
                return t;
        }
        throw new Error("Shouldn't get here!");
    }

    public void fillUniform(final float[] d, final int start, final int length) {
        for (int i = 0; i < length; i++) {
            d[start + i] = nextFloat();
        }
    }

    public void fillBinary(final float[] d, final int start, final int length) {
        for (int i = 0; i < length; i++) {
            d[start + i] = r(2);
        }
    }

    public void fillGaussian(final float[] d, final int start, final int length, final float u, final float sd)
    {
        for (int i = 0; i < length; i++) {
            d[start + i] = (float)n(u, sd);
        }
    }

    public void fillGaussian(final double[] d, final int start, final int length, final double u, final double sd)
    {
        for (int i = 0; i < length; i++) {
            d[start + i] = n(u, sd);
        }
    }

    public void binarySample(final float[] temp, final int offset, final int length) {
        for (int i = offset; i < (offset + length); i++) {
            temp[i] = (nextFloat() < temp[i]) ? 1f : 0f;
        }
    }
}
