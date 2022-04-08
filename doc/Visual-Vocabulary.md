# Visual Vocabulary

The [`relja_retrival`](https://gitlab.com/vgg/vise/-/tree/master/src/search_engine/relja_retrival) search engine in VISE uses a visual vocabulary to efficiently represent the visual content of an image. This visual vocabulary is computed using [k-means](https://en.wikipedia.org/wiki/K-means_clustering) clustering method applied to a large set of feature descriptors (e.g. 1,000,000 SIFT feature descriptors) extracted from images. The clusters obtained by k-means denotes highly populated regions in the feature space and these regions are used by the [`relja_retrival`](https://gitlab.com/vgg/vise/-/tree/master/src/search_engine/relja_retrival) search engine as anchors to efficiently represent the visual content of an image.

We recommend generating visual vocabulary from a subset of images contained in a VISE project. This often results in best image retrieval performance. However, the process of generating visual vocabulary can take several minutes. Often we want to quickly review the visual search performance on an image dataset. For such cases, one can use our generic visual vocabulary (described in next section) computed using a random set of images taken from standard image datasets. This avoids the need to compute visual vocabulary by using an existing pre-computed generic visual vocabulary. While the visual search performance may not be optimal, it provides a quick path to visual search engine for a set of images. Such visual search engine can be used to decide if it is reasonable to invest more computation time in generating a search engine based on visual vocabulary computed from images in the project.

## Generic Visual Vocabulary
We collect a random sample of images from the following image datasets:
  * [Art UK](https://artuk.org/) : random sample of 5k images
  * [British Library 1 Million](https://www.flickr.com/photos/britishlibrary/) : random sample of 5k images
  * [FLEURON](https://compositor.bham.ac.uk/) : random sample of 10k images
  * [ICTest](https://labs.brill.com/ictestset/) : random sample of 5k images
  * [NLS Chapbooks](https://data.nls.uk/data/digitised-collections/chapbooks-printed-in-scotland/) : 3629 images containing illustrations
  * [MS COCO](cocodataset.org/) : random sample of 5k images
  * Some random photographs from personal collection

We want to create a generic visual vocabulary that delivers optimal image retrieval performance. Therefore, we need to find the optimal value of the following parameters:
  * Number of clusters (k) : 10000, 50000, 100000
  * Image Size : 400x400, 800x800
  * Hamming Embedding: 32, 64

We use the [Oxford Buildings](https://www.robots.ox.ac.uk/~vgg/data/oxbuildings/) image dataset and its ground truth annotations to compute image retrieval performance under all the above settings for 30 iterations of k-means clustering.

```
+--------------+--------------+--------------+----------+
| Image Size   | Clusters     | Hamm Bits    | mAP      |
+--------------+--------------+--------------+----------+
| 800x800      | 10k          | 64           | 0.7332   |
| 800x800      | 10k          | 32           | 0.6874   |
| 800x800      | 50k          | 64           | 0.7272   |
| 800x800      | 50k          | 32           | 0.7056   |
| 800x800      | 100k         | 64           | 0.7104   |
| 800x800      | 100k         | 32           | 0.6811   |
| 400x400      | 10k          | 64           | 0.7383*  |
| 400x400      | 10k          | 32           | 0.6993   |
| 400x400      | 50k          | 64           | 0.7209   |
| 400x400      | 50k          | 32           | 0.6899   |
| 400x400      | 100k         | 64           | 0.7159   |
| 400x400      | 100k         | 32           | 0.6882   |
+--------------+--------------+--------------+----------+
```

The following configuration provides best image retrieval performance of mAP=0.7383
  * Image Size: 400x400
  * Number of clusters: 10000
  * Hamming embedding: 64
 
The resulting generic visual vocabulary is shipped with the VISE software so that it can be used for any new project. This generic visual vocabulary is also available for [download](https://www.robots.ox.ac.uk/~vgg/software/vise/download/2.x.y/relja_retrival/generic-visual-vocabulary/imcount53629).

***

Contact [Abhishek Dutta](mailto:adutta@robots.ox.ac.uk) for queries and feedback related to this page.