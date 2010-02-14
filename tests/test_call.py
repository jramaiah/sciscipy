import unittest

from numpy.testing import assert_almost_equal
from scilab import Scilab
import sciscipy 

class test_call(unittest.TestCase):
	def setUp(self):
		self.sci = Scilab()

	def test_spec(self):
		""" [test_call] Testing spec
		"""
		spec1 = self.sci.spec([[1, 2],[3, 4]])
		sciscipy.eval("spec1 = spec([1,2;3,4])")
		spec2 = sciscipy.read("spec1")
		assert_almost_equal(spec1, spec2)

	def test_mean(self):
		""" [test_call] Testing mean
		"""
		mean1 = self.sci.mean([[1, 2],[3, 4]])
		sciscipy.eval("mean1 = mean([1,2;3,4])")
		mean2 = sciscipy.read("mean1")
		comp = mean1 == mean2
		print comp
		assert(comp)

	def test_strcat(self):
		""" [test_call] Testing strcat
		"""
		strcat1 = self.sci.strcat(["1", "4"], "x")
		sciscipy.eval("strcat1 = strcat(['1', '4'], 'x')")
		strcat2 = sciscipy.read("strcat1")
		comp = strcat1 == strcat2
		print comp
		assert(comp)


	def test_length(self):
		""" [test_call] Testing length
		"""
		strlength1 = self.sci.length(["3ch","5char","plenty of char"])
		sciscipy.eval("strlength = length(['3ch','5char','plenty of char'])")
		strlength2 = sciscipy.read("strlength")
		assert_almost_equal(strlength1, strlength2)



if __name__ == '__main__':
	unittest.main()




